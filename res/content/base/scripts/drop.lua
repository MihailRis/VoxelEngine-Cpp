function on_despawn()
    print("despawn")
end

function on_grounded()
    entity.transform:set_rot(mat4.rotate({0, 1, 0}, math.random()*360))
    print(stdcomp)
    entity:despawn()
end
