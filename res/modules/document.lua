local Element = {}
function Element.new(docname, name)
    return setmetatable({docname=docname, name=name}, {
        __index=function(self, k)
            return gui.getattr(self.docname, self.name, k)
        end,
        __newindex=function(self, k, v)
            gui.setattr(self.docname, self.name, k, v)
        end
    })
end

local Document = {}
function Document.new(docname)
    return setmetatable({name=docname}, {
        __index=function(self, k)
            return Element.new(self.name, k)
        end
    })
end

return Document
