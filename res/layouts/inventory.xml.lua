function on_open(inv)
    print("OPEN", inv)
end

function on_close(inv)
    print("CLOSE", inv)
end

function inventory_share_func(invid, slotid)
    inventory.set(invid, slotid, 0, 0)
end
