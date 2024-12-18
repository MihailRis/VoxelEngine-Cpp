test.set_setting("chunks.load-distance", 3)
test.set_setting("chunks.load-speed", 1)

test.reconfig_packs({"base"}, {})
test.new_world("demo", "2019", "core:default")

local pid1 = player.create("Xerxes")
assert(player.get_name(pid1) == "Xerxes")

local pid2 = player.create("Segfault")
assert(player.get_name(pid2) == "Segfault")

local seed = math.floor(math.random() * 1e6)
print("random seed", seed)
math.randomseed(seed)

for i=1,25 do
    if i % 5 == 0 then
        print(tostring(i*4).." % done")
        print("chunks loaded", world.count_chunks())
    end
    player.set_pos(pid1, math.random() * 100 - 50, 100, math.random() * 100 - 50)
    player.set_pos(pid2, math.random() * 200 - 100, 100, math.random() * 200 - 100)
    test.tick()
end

test.close_world(true)
