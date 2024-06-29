inair = true
function on_grounded()
    entity.transform:set_rot(mat4.rotate({0, 1, 0}, math.random()*360))
    inair = false
end

function on_update()
    if inair then
        entity.transform:set_rot(mat4.rotate(entity.transform:get_rot(), {0, 1, 0}, math.random()*32))
    end
end
