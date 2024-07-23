local function setup(id, rig, index)
    rig:set_model(index, "drop-block")
    local icon = item.icon(id)
    local size = {1.0, 1.0, 1.0}
    if icon:find("^block%-previews%:") then
        local bid = block.index(icon:sub(16))
        model = block.get_model(bid)
        if model == "X" then
            size = {1.0, 0.3, 1.0}
            rig:set_model(index, "drop-item")
            rig:set_texture("$0", icon)
        else
            if model == "aabb" then
                local rot = block.get_rotation_profile(bid) == "pipe" and 4 or 0
                size = block.get_hitbox(bid, rot)[2]
                vec3.mul(size, 2.0, size)
            end
            local textures = block.get_textures(bid)
            for i,t in ipairs(textures) do
                rig:set_texture("$"..tostring(i-1), "blocks:"..textures[i])
            end
        end
    else
        size = {1.0, 0.3, 1.0}
        rig:set_model(index, "drop-item")
        rig:set_texture("$0", icon)
    end
    return size
end

return {
    setup=setup,
}
