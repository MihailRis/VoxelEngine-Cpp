function on_block_broken(id, x, y, z, playerid)
    if gfx then
        gfx.particles.emit({x+0.5, y+0.5, z+0.5}, 64, {
            lifetime=1.0,
            spawn_interval=0.0001,
            explosion={4, 4, 4},
            texture="blocks:"..block.get_textures(id)[1],
            random_sub_uv=0.1,
            size={0.1, 0.1, 0.1},
            spawn_shape="box",
            spawn_spread={0.4, 0.4, 0.4}
        })
    end

    rules.create("do-loot-non-player", true)
end
