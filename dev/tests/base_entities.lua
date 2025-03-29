local util = require "core:tests_util"

-- Create world and prepare settings
util.create_demo_world("core:default")
app.set_setting("chunks.load-distance", 3)
app.set_setting("chunks.load-speed", 1)

local base_util = require "base:util"

-- Create player
local pid = player.create("Xerxes")
player.set_spawnpoint(pid, 0, 100, 0)
player.set_pos(pid, 0, 100, 0)

-- Wait for chunk to load
app.sleep_until(function () return block.get(0, 0, 0) ~= -1 end)

-- Place a falling block
block.place(0, 2, 0, block.index("base:sand"), 0, pid)
app.tick()

-- Check if the block is falling
assert(block.get(0, 2, 0) == 0)

-- Wait for the block to fall
app.sleep_until(function () return block.get(0, 1, 0) == block.index("base:sand") end, 100)

local drop = base_util.drop({player.get_pos(pid)}, item.index("base:bazalt_breaker"), 11)
assert(drop ~= nil)
assert(drop.rigidbody ~= nil)
