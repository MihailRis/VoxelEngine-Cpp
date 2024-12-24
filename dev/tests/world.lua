-- Create/close/open/close world

-- Open
test.new_world("demo", "2019", "core:default")
assert(world.is_open())
assert(world.get_generator() == "core:default")
test.sleep(1)
assert(world.get_total_time() > 0.0)
print(world.get_total_time())

-- Close
test.close_world(true)
assert(not world.is_open())

-- Reopen
test.open_world("demo")
assert(world.is_open())
assert(world.get_total_time() > 0.0)
assert(world.get_seed() == 2019)
test.tick()

-- Close
test.close_world(true)
