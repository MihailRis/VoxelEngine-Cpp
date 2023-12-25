function on_random_update(x, y, z)
    local dirtid = block_index('base:dirt');
    if is_solid_at(x, y+1, z) then
        set_block(x, y, z, dirtid)
    else
        local grassblockid = block_index('base:grass_block')
        for lx=-1,1 do
            for ly=-1,1 do
                for lz=-1,1 do
                    if get_block(x + lx, y + ly, z + lz) == dirtid then
                        if not is_solid_at(x + lx, y + ly + 1, z + lz) then
                            set_block(x + lx, y + ly, z + lz, grassblockid)
                            return
                        end
                    end
                end
            end
        end
    end
end
