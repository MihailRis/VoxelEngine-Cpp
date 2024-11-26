function inventory_share_func(invid, slotid)
    local blockinv = hud.get_block_inventory()
    if blockinv ~= 0 then
        inventory.move(invid, slotid, blockinv)
    elseif rules.get("allow-content-access") then
        inventory.set(invid, slotid, 0, 0)
    else
        inventory.move(invid, slotid, invid)
    end
end
