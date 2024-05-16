function setup_variables()
    local x,y,z = player.get_pos(hud.get_player())
    console.set('pos.x', x)
    console.set('pos.y', y)
    console.set('pos.z', z)
end

function submit(text)
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
