-- Standard components OOP wrappers (__index tables of metatables)

local Transform = {__index={
    get_pos=function(self) return __transform.get_pos(self.eid) end,
    set_pos=function(self, v) return __transform.set_pos(self.eid, v) end,
    get_rot=function(self) return __transform.get_rot(self.eid) end,
    set_rot=function(self, m) return __transform.set_rot(self.eid, m) end,
}}

function new_Transform(eid)
    return setmetatable({eid=eid}, Transform)
end

local Rigidbody = {__index={
    is_enabled=function(self) return __rigidbody.is_enabled(self.eid) end,
    set_enabled=function(self, f) return __rigidbody.set_enabled(self.eid, f) end,
    get_vel=function(self) return __rigidbody.get_vel(self.eid) end,
    set_vel=function(self, v) return __rigidbody.set_vel(self.eid, v) end,
    get_size=function(self) return __rigidbody.get_size(self.eid) end,
}}

function new_Rigidbody(eid)
    return setmetatable({eid=eid}, Rigidbody)
end

-- Entity class

local Entity = {__index={
    despawn=function(self) return entity.despawn(self.eid) end,
}}

local entities = {}

return {
    new_Entity = function(eid)
        local entity = setmetatable({eid=eid}, Entity)
        entity.transform = new_Transform(eid)
        entity.rigidbody = new_Rigidbody(eid)
        entities[eid] = entity;
        return entity
    end,
    remove_Entity = function(eid)
        local entity = entities[eid]
        if entity and entity.on_despawn then
            entity.on_despawn()
        end
        entities[eid] = nil;
    end
}
