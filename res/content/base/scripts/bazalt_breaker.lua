function on_block_break_by(x, y, z, pid)
    block.set(x, y, z, 0, 0)
    inventory.use(player.get_inventory(pid))
end
