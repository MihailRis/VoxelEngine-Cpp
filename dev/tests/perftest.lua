app.set_setting("chunks.load-distance", 3)
app.set_setting("chunks.load-speed", 16)

app.reconfig_packs({"base"}, {})
app.new_world("demo", "2019", "core:default")

local pid = player.create("A")
assert(player.get_name(pid) == "A")
app.tick()

timeit(1e7, block.get, 0, 0, 0)
timeit(1e7, block.set, 0, 0, 0, 6)
