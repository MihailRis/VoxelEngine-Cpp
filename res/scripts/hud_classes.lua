local Text3D = {__index={
    hide=function(self) return gfx.text3d.hide(self.id) end,
    get_pos=function(self) return gfx.text3d.get_pos(self.id) end,
    set_pos=function(self, v) return gfx.text3d.set_pos(self.id, v) end,
    get_text=function(self) return gfx.text3d.get_text(self.id) end,
    set_text=function(self, s) return gfx.text3d.set_text(self.id, s) end,
    update_settings=function(self, t) return gfx.text3d.update_settings(self.id, t) end,
}}

gfx.text3d.new = function(pos, text, preset, extension)
    local id = gfx.text3d.show(pos, text, preset, extension)
    return setmetatable({id=id}, Text3D)
end
