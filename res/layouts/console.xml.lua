history = session.get_entry("commands_history")
history_pointer = #history

function setup_variables()
    local x,y,z = player.get_pos(hud.get_player())
    console.set('pos.x', x)
    console.set('pos.y', y)
    console.set('pos.z', z)
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
    if history_pointer == #history-1 then
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
    
    local status, result = pcall(function() return console.execute(text) end)
    if result ~= nil then
        local prevtext = document.log.text
        if #prevtext == 0 then
            document.log:paste(tostring(result))
        else
            document.log:paste('\n'..tostring(result))
        end
    end
    document.prompt.text = ""
    document.prompt.focused = true
end

function on_open()
    document.prompt.focused = true
end
