function inventory_share_func(invid, slotid)
    local blockinv = hud.get_block_inventory()
    if blockinv ~= 0 then
        inventory.move(invid, slotid, blockinv)
    else
        inventory.set(invid, slotid, 0, 0)
    end
end
