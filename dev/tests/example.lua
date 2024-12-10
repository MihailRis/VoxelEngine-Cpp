test.new_world("demo", "2019", "core:default")
assert(world.get_generator() == "core:default")
test.close_world(true)
assert(not world.is_open())
