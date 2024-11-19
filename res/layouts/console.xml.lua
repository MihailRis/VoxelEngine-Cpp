console_mode = "console"

history = session.get_entry("commands_history")
history_pointer = #history

local warnings_all = {}
local errors_all = {}

local warning_id = 0
local error_id = 0

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
                callback = "local source = file.read('"..frame.source.."') "..
                           "document.editor.text = source "..
                           "document.editor.focused = true "..
                           "time.post_runnable(function() document.editor.caret = document.editor:linePos("..
                           tostring(frame.currentline-1)..") end)"
            else
                callback = "document.editor.text = 'Could not open source file'"
            end
            callback = callback.." document.title.text = gui.str('File')..' - "
                       ..frame.source.."'"
        end
        if frame.name then
            framestr = framestr.."("..tostring(frame.name)..")"
        end
        local color = "#FFFFFF"
        if frame.source:starts_with("core:") then
            color = "#C0D0C5"
        end
        tb_list:add(gui.template("stack_frame", {
            location=framestr, 
            color=color,
            callback=callback
        }))
    end
    tb_list.size = srcsize
end)

function setup_variables()
    local pid = hud.get_player()
    local x,y,z = player.get_pos(pid)
    console.set('pos.x', x)
    console.set('pos.y', y)
    console.set('pos.z', z)
    local pentity = player.get_entity(pid)
    if pentity ~= 0 then
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
    text = text:trim()
    add_to_history(text)

    if console_mode == "chat" then
        if not text:starts_with("/") then
            text = "chat "..string.escape(text)
        else
            text = text:sub(2)
        end
    end

    setup_variables()
    
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
    local status, result = pcall(console.execute, text)
    if result then
        console.log(result)
    end
    document.prompt.text = ""
    document.prompt.focused = true
end

function set_mode(mode)
    local show_prompt = mode == 'chat' or mode == 'console'

    document.title.text = ""
    document.editorContainer.visible = mode == 'debug'
    document.logContainer.visible = mode ~= 'debug'

    if mode == 'debug' then
        document.root.color = {16, 18, 20, 220}
    else
        document.root.color = {0, 0, 0, 128}
    end

    document.traceback.visible = mode == 'debug'
    document.prompt.visible = show_prompt
    if show_prompt then
        document.prompt.focused = true
    end
    console_mode = mode
end

function on_open()
    if modes == nil then
        modes = RadioGroup({
            chat=document.s_chat,
            console=document.s_console,
            debug=document.s_debug
        }, function (mode)
            set_mode(mode)
        end, "console")
    end
end
