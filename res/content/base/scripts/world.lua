function on_block_broken(id, x, y, z, playerid)
    particles.emit({x+0.5, y+0.5, z+0.5}, 100, {
        lifetime=2.0,
        spawn_interval=0.0001,
        explosion={4, 4, 4},
        texture="blocks:"..block.get_textures(id)[1],
        random_sub_uv=0.1
    })
end
