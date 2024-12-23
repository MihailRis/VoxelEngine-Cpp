local base_util = require "util"

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
        local drop = base_util.drop(ppos, itemid, 1, 1.5)
        local velocity = vec3.add(throw_force, vec3.add(pvel, DROP_INIT_VEL))
        drop.rigidbody:set_vel(velocity)
    end)
    input.add_callback("player.pick", function ()
        if hud.is_paused() or hud.is_inventory_open() then
            return
        end
        local pid = hud.get_player()
        local x, y, z = player.get_selected_block(pid)
        if x == nil then
            return
        end
        local id = block.get_picking_item(block.get(x, y, z))
        local inv, cur_slot = player.get_inventory(pid)
        local slot = inventory.find_by_item(inv, id, 0, 9)
        if slot then
            player.set_selected_slot(pid, slot)
            return
        end
        if not rules.get("allow-content-access") then
            return
        end
        slot = inventory.find_by_item(inv, 0, 0, 9)
        if slot then
            cur_slot = slot
        end
        player.set_selected_slot(pid, cur_slot)
        inventory.set(inv, cur_slot, id, 1)
    end)
end
