local DROP_FORCE = 8
local DROP_INIT_VEL = {0, 3, 0}

local note

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

    note = gfx.text3d.new({0.5, 99.5, 0.0015}, "Segmentation fault", {
        scale=0.005,
        color={0, 0, 0, 1},
        display="static_billboard",
        xray_opacity=0.2
    })
end

function on_hud_render()
    note:update_settings({
        color={math.sin(time.uptime() * 12) * 0.5 + 0.5, 0, 0, 1}
    })
    -- note:set_axis_x({math.sin(time.uptime()), 0, math.cos(time.uptime())})
    note:set_rotation(mat4.rotate({1, 1, 0}, time.uptime() * 57))
end
