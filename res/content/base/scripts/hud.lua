local DROP_FORCE = 8
local DROP_INIT_VEL = {0, 3, 0}

function on_hud_open()
    input.add_callback("player.drop", function ()
        if hud.is_paused() or hud.is_inventory_open() then
            return
        end
        local pid = hud.get_player()
        local invid, slot = player.get_inventory(pid)
        local itemid, itemcount = inventory.get(invid, slot)
        if itemid == 0 then
            return
        end
        inventory.set(invid, slot, itemid, itemcount-1)

        local pvel = {player.get_vel(pid)}
        local ppos = vec3.add({player.get_pos(pid)}, {0, 0.7, 0})
        local throw_force = vec3.mul(player.get_dir(pid), DROP_FORCE)
        local drop = entities.spawn("base:drop", ppos, {base__drop={
            id=itemid,
            count=1
        }})
        local velocity = vec3.add(throw_force, vec3.add(pvel, DROP_INIT_VEL))
        drop.rigidbody:set_vel(velocity)
    end)
end
