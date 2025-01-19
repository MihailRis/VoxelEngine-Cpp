local lines = {}
local dead_lines = {}
local nextid = 0
local timeout = 7
local fadeout = 1
local initialized = false
local max_lines = 15
local animation_fps = 30

local function remove_line(line)
    document[line[1]]:destruct()
    time.post_runnable(function() document.root:reposition() end)
end

local function update_line(line, uptime)
    local diff = uptime - line[2]
    if diff > timeout then
        remove_line(line)
        table.insert(dead_lines, i)
    elseif diff > timeout-fadeout then
        local opacity = (timeout - diff) / fadeout
        document[line[1]].color = {0, 0, 0, opacity * 80}
        document[line[1].."L"].color = {255, 255, 255, opacity * 255}
    end
end

events.on("core:chat", function(message)
    local current_time = time.uptime()
    local id = 'l'..tostring(nextid)
    document.root:add(gui.template("chat_line", {id=id}))
    document.root:reposition()
    document[id.."L"].text = message
    nextid = nextid + 1
    if #lines == max_lines then
        remove_line(lines[1])
        table.remove(lines, 1)
    end
    table.insert(lines, {id, current_time})
end)

function on_open()
    if not initialized then
        initialized = true
        
        document.root:setInterval(1/animation_fps * 1000, function ()
            local uptime = time.uptime()
            for _, line in ipairs(lines) do
                update_line(line, uptime)
            end
            if #dead_lines > 0 then
                for i = #dead_lines, 1, -1 do
                    local index = dead_lines[i]
                    table.remove(lines, i)
                end
                dead_lines = {}
            end
        end)
    end
end