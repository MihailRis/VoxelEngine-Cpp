local user_props = file.read_combined_object("config/user-props.toml")
local names = {}
for name, _ in pairs(user_props) do
    table.insert(names, name)
end
-- remove undefined properties
for id, blockprops in pairs(block.properties) do
    for propname, value in pairs(blockprops) do
        if propname:find(':') and not table.has(names, propname) then
            print("erase property", propname)
            blockprops[propname] = nil
        end
    end
end

local function make_read_only(t)
    setmetatable(t, {
        __newindex = function()
            error("table is read-only")
        end
    })    
end

make_read_only(block.properties)
for k,v in pairs(block.properties) do
    make_read_only(v)
end
