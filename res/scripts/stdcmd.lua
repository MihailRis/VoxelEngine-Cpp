console.add_command(
    "tp obj:sel=$obj.id x:num~pos.x y:num~pos.y z:num~pos.z",
    function (args, kwargs)
        player.set_pos(unpack(args))
    end
)
console.add_command(
    "echo value:str",
    function (args, kwargs)
        return args[1]
    end
)
console.add_command(
    "time.set value:num",
    function (args, kwargs)
        return world.set_day_time(args[1])
    end
)
console.add_command(
    "fill id:str x:num~pos.x y:num~pos.y z:num~pos.z w:int h:int d:int",
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
