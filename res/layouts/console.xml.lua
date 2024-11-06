history = session.get_entry("commands_history")
history_pointer = #history

local warning_id = 0
events.on("core:warning", function (wtype, text)
    document.problemsLog:add(gui.template("problem", {
        type="warning", text=wtype..": "..text, id=tostring(warning_id)
    }))
    warning_id = warning_id + 1
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
    add_to_history(text)
    setup_variables()
    
    text = text:trim()
    local name
    for s in text:gmatch("%S+") do
        name = s
        break
    end
    if name == nil then
        name = text
    end
    if not rules.get("cheat-commands") and table.has(console.cheats, name) then
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

function on_open()
    document.prompt.focused = true
end
