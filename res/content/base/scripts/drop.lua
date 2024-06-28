function on_spawn(eid)
    print("spawn", eid)
end

function on_despawn(eid)
    print("despawn", eid)
end

function on_grounded(eid)
    local entity = stdcomp.new_Entity(eid) -- test
    entity.transform:set_rot(mat4.rotate({0, 1, 0}, math.random()*360))
end
