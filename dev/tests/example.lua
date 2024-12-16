test.set_setting("chunks.load-distance", 3)
test.set_setting("chunks.load-speed", 16)

test.reconfig_packs({"base"}, {})
test.new_world("demo", "2019", "core:default")
local pid = player.create("Xerxes")
assert(player.get_name(pid) == "Xerxes")
test.sleep_until(function() return world.count_chunks() >= 9 end, 1000)
print(world.count_chunks())

for i=1,10 do
    print("--------------------")
    timeit(10000000, block.get_fast, 0, 0, 0)
    timeit(10000000, block.get, 0, 0, 0)
end

block.destruct(0, 0, 0, pid)
assert(block.get(0, 0, 0) == 0)
test.close_world(true)
