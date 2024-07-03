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

local Modeltree = {__index={
    get_model=function(self, i) return __modeltree.get_model(self.eid, i) end,
    get_matrix=function(self, i) return __modeltree.get_matrix(self.eid, i) end,
    set_matrix=function(self, i, m) return __modeltree.set_matrix(self.eid, i, m) end,
    set_texture=function(self, s, s2) return __modeltree.set_texture(self.eid, s, s2) end,
}}

function new_Modeltree(eid)
    return setmetatable({eid=eid}, Modeltree)
end

-- Entity class

local Entity = {__index={
    despawn=function(self) return entities.despawn(self.eid) end,
}}

local entities = {}

return {
    new_Entity = function(eid)
        local entity = setmetatable({eid=eid}, Entity)
        entity.transform = new_Transform(eid)
        entity.rigidbody = new_Rigidbody(eid)
        entity.modeltree = new_Modeltree(eid)
        entity.data = {}
        entities[eid] = entity;
        return entity
    end,
    get_Entity = function(eid)
        return entities[eid]
    end,
    remove_Entity = function(eid)
        local entity = entities[eid]
        if entity then
            entity.env = nil
            entities[eid] = nil;
        end
    end,
    update = function()
        for id,entity in pairs(entities) do
            local callback = entity.env.on_update
            if callback then
                local result, err = pcall(callback)
                if err then
                    print(err)
                end
            end
        end
    end
}
