console_mode = "console"

history = session.get_entry("commands_history")
history_pointer = #history

local warnings_all = {}
local errors_all = {}

local warning_id = 0
local error_id = 0

local writeables = {}

local current_file = {
    filename = "",
    mutable = nil
}

events.on("core:warning", function (wtype, text, traceback)
    local full = wtype..": "..text
    if table.has(warnings_all, full) then
        return
    end
    local encoded = base64.encode(bjson.tobytes({frames=traceback}))
    document.problemsLog:add(gui.template("problem", {
        type="warning", 
        text=full, 
        traceback=encoded, 
        id=tostring(warning_id)
    }))
    warning_id = warning_id + 1
    table.insert(warnings_all, full)
end)

events.on("core:error", function (msg, traceback)
    local _, endindex = string.find(msg, ": ")
    local full = ""
    for i,frame in ipairs(traceback) do
        full = full..frame.source..tostring(frame.currentline)
    end
    if table.has(errors_all, full) then
        return
    end
    local encoded = base64.encode(bjson.tobytes({frames=traceback}))
    document.problemsLog:add(gui.template("problem", {
        type="error", 
        text=msg:sub(endindex), 
        traceback=encoded,
        id=tostring(error_id)
    }))
    error_id = error_id + 1
    table.insert(errors_all, full)
end)

local function find_mutable(filename)
    local saved = writeables[file.prefix(filename)]
    if saved then
        return saved..":"..file.path(filename)
    end

    local packid = file.prefix(filename)
    local packinfo = pack.get_info(packid)
    if not packinfo then
        return
    end
    local path = packinfo.path
    if file.is_writeable(path) then
        return file.join(path, file.path(filename))
    end
end

local function refresh_file_title()
    if current_file.filename == "" then
        document.title.text = ""
        return
    end
    local edited = document.editor.edited
    current_file.modified = edited
    document.saveIcon.enabled = edited
    document.title.text = gui.str('File')..' - '..current_file.filename
        ..(edited and ' *' or '')
end

function on_control_combination(keycode)
    if keycode == input.keycode("s") then
        save_current_file()
    end
end

function unlock_access()
    if current_file.filename == "" then
        return
    end
    pack.request_writeable(file.prefix(current_file.filename), 
        function(token)
            writeables[file.prefix(current_file.filename)] = token
            current_file.mutable = token..":"..file.path(current_file.filename)
            open_file_in_editor(current_file.filename, 0, current_file.mutable)
        end
    )
end

function save_current_file()
    if not current_file.mutable then
        return
    end
    file.write(current_file.mutable, document.editor.text)
    current_file.modified = false
    document.saveIcon.enabled = false
    document.title.text = gui.str('File')..' - '..current_file.filename
    document.editor.edited = false
end

function open_file_in_editor(filename, line, mutable)
    local editor = document.editor
    local source = file.read(filename):gsub('\t', '    ')
    editor.text = source
    editor.focused = true
    if line then
        time.post_runnable(function()
            editor.caret = editor:linePos(line)
        end)
    end
    document.title.text = gui.str('File') .. ' - ' .. filename
    current_file.filename = filename
    current_file.mutable = mutable or find_mutable(filename)
    document.lockIcon.visible = current_file.mutable == nil
    document.editor.editable = current_file.mutable ~= nil
    document.saveIcon.enabled = current_file.modified
end

events.on("core:open_traceback", function(traceback_b64)
    local traceback = bjson.frombytes(base64.decode(traceback_b64))
    modes:set('debug')

    local tb_list = document.traceback
    local srcsize = tb_list.size
    tb_list:clear()
    tb_list:add("<label enabled='false' margin='2'>@devtools.traceback</label>")
    for _, frame in ipairs(traceback.frames) do
        local callback = ""
        local framestr = ""
        if frame.what == "C" then
            framestr = "C/C++ "
        else
            framestr = frame.source..":"..tostring(frame.currentline).." "
            if file.exists(frame.source) then
                callback = string.format(
                    "open_file_in_editor('%s', %s)",
                    frame.source, frame.currentline-1
                )
            else
                callback = "document.editor.text = 'Could not open source file'"
            end
        end
        if frame.name then
            framestr = framestr.."("..tostring(frame.name)..")"
        end
        local color = "#FFFFFF"
        tb_list:add(gui.template("stack_frame", {
            location=framestr, 
            color=color,
            callback=callback,
            enabled=file.exists(frame.source)
        }))
    end
    tb_list.size = srcsize
end)

function setup_variables()
    local pid = hud.get_player()
    local x,y,z = player.get_pos(pid)
    console.set("player", pid)
    console.set('pos.x', x)
    console.set('pos.y', y)
    console.set('pos.z', z)
    local pentity = player.get_entity(pid)
    if pentity > 0 then
        console.set('entity.id', pentity)
    end
    local sentity = player.get_selected_entity(pid)
    if sentity ~= nil then
        console.set('entity.selected', sentity)
    end
end

function on_history_up()
    if history_pointer == 0 then
        return
    end
    document.prompt.text = history[history_pointer]
    document.prompt.caret = -1
    history_pointer = history_pointer - 1
end

function on_history_down()
    if history_pointer >= #history-1 then
        return
    end
    history_pointer = history_pointer + 1
    document.prompt.text = history[history_pointer + 1]
    document.prompt.caret = -1
end

function add_to_history(text)
    table.insert(history, text)
    history_pointer = #history
end

function submit(text)
    if #text == 0 then
        document.prompt.focused = true
        return
    end
    text = text:trim()
    add_to_history(text)

    if console_mode == "chat" then
        if not text:starts_with("/") then
            text = "chat "..string.escape(text)
        else
            text = text:sub(2)
        end
    end
    
    local name
    for s in text:gmatch("%S+") do
        name = s
        break
    end
    if name == nil then
        name = text
    end
    if not rules.get("allow-cheats") and table.has(console.cheats, name) then
        console.log("cheat commands are disabled")
        document.prompt.text = ""
        document.prompt.focused = true
        return
    end
    
    document.log.caret = -1
    document.prompt.text = ""
    document.prompt.focused = true

    setup_variables()

    if console.submit then
        console.submit(text)
    else    
        local status, result = pcall(console.execute, text)
        if result then
            console.log(result)
        end
    end
end

function set_mode(mode)
    local show_prompt = mode == 'chat' or mode == 'console'

    document.lockIcon.visible = false
    document.editorRoot.visible = mode == 'debug'
    document.editorContainer.visible = mode == 'debug'
    document.logContainer.visible = mode ~= 'debug'

    if mode == 'debug' then
        document.root.color = {16, 18, 20, 220}
    else
        document.root.color = {0, 0, 0, 128}
    end

    document.prompt.visible = show_prompt
    if show_prompt then
        document.prompt.focused = true
    end
    console_mode = mode
end

local function collect_scripts(dirname, dest)
    if file.isdir(dirname) then
        local files = file.list(dirname)
        for i, filename in ipairs(files) do
            if file.isdir(filename) then
                collect_scripts(filename, dest)
            elseif file.ext(filename) == "lua" then
                table.insert(dest, filename)
            end
        end
    end
end

function on_open(mode)
    if modes == nil then
        modes = RadioGroup({
            chat=document.s_chat,
            console=document.s_console,
            debug=document.s_debug
        }, function (mode)
            set_mode(mode)
        end, mode or "console")

        local files_list = document.filesList
        local packs = pack.get_installed()

        local scripts = {}
        for _, packid in ipairs(packs) do
            collect_scripts(packid..":modules", scripts)
            collect_scripts(packid..":scripts", scripts)
        end
        table.sort(scripts)
        for _, filename in ipairs(scripts) do
            local parent = file.parent(filename)
            local script_type = "file"
            files_list:add(gui.template("script_file", {
                path = parent .. (parent[#parent] == ':' and '' or '/'), 
                name = file.name(filename),
                type = script_type,
                filename = filename
            }))
        end

        document.editorContainer:setInterval(200, refresh_file_title)
    elseif mode then
        modes:set(mode)
    end
    hud.close("core:ingame_chat")
end

function on_close()
    time.post_runnable(function()
        hud.open_permanent("core:ingame_chat")
    end)
end
