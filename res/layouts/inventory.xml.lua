function on_open(invid)
    print("OPEN", invid)
end

function on_close(invid)
    print("CLOSE", invid)
end

function inventory_share_func(invid, slotid)
    inventory.set(invid, slotid, 0, 0)
end
