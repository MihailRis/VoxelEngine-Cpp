test.set_setting("chunks.load-distance", 3)
test.set_setting("chunks.load-speed", 32)

test.reconfig_packs({"base"}, {})
test.new_world("demo", "2019", "core:default")
local pid = player.create("Xerxes")
assert(player.get_name(pid) == "Xerxes")

for i=1,100 do
    if i % 10 == 0 then
        print(tostring(i).." % done")
        print("chunks loaded", world.count_chunks())
    end
    player.set_pos(pid, math.random() * 100 - 50, 100, math.random() * 100 - 50)
    test.tick()
end

test.close_world(true)
