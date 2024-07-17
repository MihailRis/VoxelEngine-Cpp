local function update(x, y, z)
    if block.is_replaceable_at(x, y-1, z) then
        entities.spawn("base:falling_block", {x+0.5, y+0.5, z+0.5}, 
            {base__falling_block={block='base:sand'}})
        block.set(x, y, z, 0)
    end
end

function on_update(x, y, z)
    update(x, y, z)
end

function on_placed(x, y, z)
    update(x, y, z)
end
