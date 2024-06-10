function on_interact(x, y, z)
    local inc = 1
    if block.get_user_bits(x, y, z, 0, 1) > 0 then
        inc = 3
        block.set_user_bits(x, y, z, 0, 1, 0)
        audio.play_sound("blocks/door_close", x+0.5, y+1, z+0.5, 1, 1)
    else
        block.set_user_bits(x, y, z, 0, 1, 1)
        audio.play_sound("blocks/door_open", x+0.5, y+1, z+0.5, 1, 1)
    end
    block.set_rotation(x, y, z, (block.get_rotation(x, y, z) + inc) % 4)
    return true
end
