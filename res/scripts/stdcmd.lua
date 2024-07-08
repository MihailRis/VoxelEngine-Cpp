local SEPARATOR = "________________"
SEPARATOR = SEPARATOR..SEPARATOR..SEPARATOR

function build_scheme(command)
    local str = command.name.." "
    for i,arg in ipairs(command.args) do
        if arg.optional then
            str = str.."["..arg.name.."] "
        else
            str = str.."<"..arg.name.."> "
        end
    end
    return str
end

console.add_command(
    "clear",
    "Clears the console",
    function ()
        local document = Document.new("core:console")
        document.log.text = ""
    end
)

console.add_command(
    "help name:str=''",
    "Show help infomation for the specified command",
    function (args, kwargs)

        local name = args[1]
        if #name == 0 then

            local commands = console.get_commands_list()
            table.sort(commands)
            local str = "Available commands:"

            for i,k in ipairs(commands) do
                str = str.."\n  "..build_scheme(console.get_command_info(k))
            end

            return str.."\nuse 'help <command>'"

        end

        local command = console.get_command_info(name)

        if command == nil then
            return string.format("command %q not found", name)
        end

        local where = ":"
        local str = SEPARATOR.."\n"..command.description.."\n"..name.." "

        for _, arg in ipairs(command.args) do
            where = where.."\n  "..arg.name.." - "..arg.type

            if arg.optional then
                str = str.."["..arg.name.."] "
                where = where.." (optional)"
            else
                str = str.."<"..arg.name.."> "
            end
        end

        if #command.args > 0 then
            str = str.."\nwhere"..where
        end

        return str.."\n"..SEPARATOR

    end
)

console.add_command(
    "time.uptime",
    "Get time elapsed since the engine started",
    function()

        local uptime = time.uptime()
        local years = math.floor(uptime / 31536000)
        local days = math.floor((uptime % 31536000) / 86400) % 365
        local hours = math.floor((uptime % 86400) / 3600) % 24
        local minutes = math.floor((uptime % 3600) / 60) % 60
        local seconds = math.floor(uptime % 60)

        local formatted_uptime = ""

        if years > 0 then
            formatted_uptime = formatted_uptime .. years .. "y "
        end
        if days > 0 or years > 0 then
            formatted_uptime = formatted_uptime .. days .. "d "
        end
        if hours > 0 or days > 0 or years > 0 then
            formatted_uptime = formatted_uptime .. hours .. "h "
        end
        if minutes > 0 or hours > 0 or days > 0 or years > 0 then
            formatted_uptime = formatted_uptime .. minutes .. "m "
        end
        if seconds > 0 or minutes > 0 or hours > 0 or days > 0 or years > 0 then
            formatted_uptime = formatted_uptime .. seconds .. "s"
        end

        return uptime .. " (" .. formatted_uptime .. ")"
    end
)

console.add_command(
    "tp obj:sel=$obj.id x:num~pos.x y:num~pos.y z:num~pos.z",
    "Teleport object",
    function (args, kwargs)
        player.set_pos(unpack(args))
    end
)
console.add_command(
    "echo value:str",
    "Print value to the console",
    function (args, kwargs)
        return args[1]
    end
)
console.add_command(
    "time.set value:num",
    "Set day time [0..1] where 0 is midnight, 0.5 is noon",
    function (args, kwargs)
        return world.set_day_time(args[1])
    end
)
console.add_command(
    "blocks.fill id:str x:num~pos.x y:num~pos.y z:num~pos.z w:int h:int d:int",
    "Fill specified zone with blocks",
    function (args, kwargs)
        local name, x, y, z, w, h, d = unpack(args)
        local id = block.index(name)
        for ly=0,h-1 do
            for lz=0,d-1 do
                for lx=0,w-1 do
                    block.set(x+lx, y+ly, z+lz, id)
                end
            end
        end
        return tostring(w*h*d).." blocks set"
    end
)
