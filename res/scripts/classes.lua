local Camera = {__index = {
    get_pos = function(self) return cameras.get_pos(self.cid) end,
    set_pos = function(self, v) return cameras.set_pos(self.cid, v) end,
    get_name = function(self) return cameras.name(self.cid) end,
    get_index = function(self) return self.cid end,
    get_rot = function(self) return cameras.get_rot(self.cid) end,
    set_rot = function(self, m) return cameras.set_rot(self.cid, m) end,
    get_zoom = function(self) return cameras.get_zoom(self.cid) end,
    set_zoom = function(self, f) return cameras.set_zoom(self.cid, f) end,
    get_fov = function(self) return cameras.get_fov(self.cid) end,
    set_fov = function(self, f) return cameras.set_fov(self.cid, f) end,
    is_perspective = function(self) return cameras.is_perspective(self.cid) end,
    set_perspective = function(self, b) return cameras.set_perspective(self.cid, b) end,
    is_flipped = function(self) return cameras.is_flipped(self.cid) end,
    set_flipped = function(self, b) return cameras.set_flipped(self.cid, b) end,
    get_front = function(self) return cameras.get_front(self.cid) end,
    get_right = function(self) return cameras.get_right(self.cid) end,
    get_up = function(self) return cameras.get_up(self.cid) end,
    look_at = function(self, v, f) return cameras.look_at(self.cid, v, f) end,
}}

local wrappers = {}

cameras.get = function(name)
    local wrapper = wrappers[name]
    if wrapper then
        return wrapper
    end
    local cid = cameras.index(name)
    wrapper = setmetatable({cid = cid}, Camera)
    wrappers[name] = wrapper
    return wrapper
end
