-- Standard components OOP wrappers (__index tables of metatables)

local Transform = {__index={
    get_pos=function(self) return __transform.get_pos(self.eid) end,
    set_pos=function(self, v) return __transform.set_pos(self.eid, v) end,
    get_size=function(self) return __transform.get_size(self.eid) end,
    set_size=function(self, v) return __transform.set_size(self.eid, v) end,
    get_rot=function(self) return __transform.get_rot(self.eid) end,
    set_rot=function(self, m) return __transform.set_rot(self.eid, m) end,
}}

local function new_Transform(eid)
    return setmetatable({eid=eid}, Transform)
end

local Rigidbody = {__index={
    is_enabled=function(self) return __rigidbody.is_enabled(self.eid) end,
    set_enabled=function(self, b) return __rigidbody.set_enabled(self.eid, b) end,
    get_vel=function(self) return __rigidbody.get_vel(self.eid) end,
    set_vel=function(self, v) return __rigidbody.set_vel(self.eid, v) end,
    get_size=function(self) return __rigidbody.get_size(self.eid) end,
    set_size=function(self, v) return __rigidbody.set_size(self.eid, v) end,
    get_gravity_scale=function(self) return __rigidbody.get_gravity_scale(self.eid) end,
    set_gravity_scale=function(self, s) return __rigidbody.set_gravity_scale(self.eid, s) end,
    get_linear_damping=function(self) return __rigidbody.get_linear_damping(self.eid) end,
    set_linear_damping=function(self, f) return __rigidbody.set_linear_damping(self.eid, f) end,
    is_vdamping=function(self) return __rigidbody.is_vdamping(self.eid) end,
    set_vdamping=function(self, b) return __rigidbody.set_vdamping(self.eid, b) end,
    is_grounded=function(self) return __rigidbody.is_grounded(self.eid) end,
    is_crouching=function(self) return __rigidbody.is_crouching(self.eid) end,
    set_crouching=function(self, b) return __rigidbody.set_crouching(self.eid, b) end,
    get_body_type=function(self) return __rigidbody.get_body_type(self.eid) end,
    set_body_type=function(self, s) return __rigidbody.set_body_type(self.eid, s) end,
}}

local function new_Rigidbody(eid)
    return setmetatable({eid=eid}, Rigidbody)
end

local Skeleton = {__index={
    get_model=function(self, i) return __skeleton.get_model(self.eid, i) end,
    get_matrix=function(self, i) return __skeleton.get_matrix(self.eid, i) end,
    set_matrix=function(self, i, m) return __skeleton.set_matrix(self.eid, i, m) end,
    set_texture=function(self, s, s2) return __skeleton.set_texture(self.eid, s, s2) end,
}}

local function new_Skeleton(eid)
    return setmetatable({eid=eid}, Skeleton)
end

-- Entity class

local Entity = {__index={
    despawn=function(self) return entities.despawn(self.eid) end,
    set_rig=function(self, s) return entities.set_rig(self.eid, s) end,
    get_component=function(self, name) return self.components[name] end,
    has_component=function(self, name) return self.components[name] ~= nil end,
    get_uid=function(self) return self.eid end,
}}

local entities = {}

return {
    new_Entity = function(eid)
        local entity = setmetatable({eid=eid}, Entity)
        entity.transform = new_Transform(eid)
        entity.rigidbody = new_Rigidbody(eid)
        entity.skeleton = new_Skeleton(eid)
        entity.components = {}
        entities[eid] = entity;
        return entity
    end,
    get_Entity = function(eid)
        return entities[eid]
    end,
    remove_Entity = function(eid)
        local entity = entities[eid]
        if entity then
            entity.components = nil
            entities[eid] = nil;
        end
    end,
    update = function()
        for _,entity in pairs(entities) do
            for _, component in pairs(entity.components) do
                local callback = component.on_update
                if callback then
                    local result, err = pcall(callback)
                    if err then
                        --// TODO: replace with error logging
                        print(err)
                    end
                end
            end
        end
    end,
    render = function()
        for _,entity in pairs(entities) do
            for _, component in pairs(entity.components) do
                local callback = component.on_render
                if callback then
                    local result, err = pcall(callback)
                    if err then
                        --// TODO: replace with error logging
                        print(err)
                    end
                end
            end
        end
    end
}
