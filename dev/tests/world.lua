-- Create/close/open/close world

-- Open
app.reconfig_packs({"base"}, {})
app.new_world("demo", "2019", "core:default")
assert(world.is_open())
assert(world.get_generator() == "core:default")
app.sleep(1)
assert(world.get_total_time() > 0.0)
print(world.get_total_time())

-- Close
app.close_world(true)
assert(not world.is_open())

-- Reopen
app.open_world("demo")
assert(world.is_open())
assert(world.get_total_time() > 0.0)
assert(world.get_seed() == 2019)
app.tick()
app.reconfig_packs({}, {"base"})
app.tick()

-- Close
app.close_world(true)
app.delete_world("demo")
