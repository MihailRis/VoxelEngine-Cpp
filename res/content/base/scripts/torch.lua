function on_placed(x, y, z)
    particles.emit({x + 0.5, y + 0.4, z + 0.5}, -1, {
        lifetime=2.0,
        spawn_interval=0.3,
        acceleration={0, 0, 0},
        velocity={0, 0.3, 0},
        explosion={0, 0, 0},
        size={0.2, 0.2, 0.2},
        spawn_shape="ball",
        spawn_spread={0.05, 0.05, 0.05},
        lighting=false,
        frames={
            "particles:fire_0",
            "particles:smoke_0",
            "particles:smoke_1"
        }
    })
end
