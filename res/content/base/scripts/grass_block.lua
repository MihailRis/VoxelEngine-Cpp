function on_random_update(x, y, z)
    local dirtid = block.index('base:dirt');
    if block.is_solid_at(x, y+1, z) then
        block.set(x, y, z, dirtid, 0)
    else
        local grassblockid = block.index('base:grass_block')
        for lx=-1,1 do
            for ly=-1,1 do
                for lz=-1,1 do
                    if block.get(x + lx, y + ly, z + lz) == dirtid then
                        if not block.is_solid_at(x + lx, y + ly + 1, z + lz) then
                            block.set(x + lx, y + ly, z + lz, grassblockid, 0)
                            return
                        end
                    end
                end
            end
        end
    end
end
