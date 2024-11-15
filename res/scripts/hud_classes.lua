local Text3D = {__index={
    hide=function(self) return gfx.text3d.hide(self.id) end,
    get_pos=function(self) return gfx.text3d.get_pos(self.id) end,
    set_pos=function(self, v) return gfx.text3d.set_pos(self.id, v) end,
    get_axis_x=function(self) return gfx.text3d.get_axis_x(self.id) end,
    set_axis_x=function(self, v) return gfx.text3d.set_axis_x(self.id, v) end,
    get_axis_y=function(self) return gfx.text3d.get_axis_y(self.id) end,
    set_axis_y=function(self, v) return gfx.text3d.set_axis_y(self.id, v) end,
    set_rotation=function(self, m) return gfx.text3d.set_rotation(self.id, m) end,
    get_text=function(self) return gfx.text3d.get_text(self.id) end,
    set_text=function(self, s) return gfx.text3d.set_text(self.id, s) end,
    update_settings=function(self, t) return gfx.text3d.update_settings(self.id, t) end,
}}

gfx.text3d.new = function(pos, text, preset, extension)
    local id = gfx.text3d.show(pos, text, preset, extension)
    return setmetatable({id=id}, Text3D)
end
