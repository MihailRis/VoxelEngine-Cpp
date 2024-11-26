function make_read_only(t)
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
