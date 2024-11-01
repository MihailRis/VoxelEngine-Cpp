------------------------------------------------
------ Extended kit of standard functions ------
------------------------------------------------

function sleep(timesec)
    local start = time.uptime()
    while time.uptime() - start < timesec do
        coroutine.yield()
    end
end

------------------------------------------------
------------------- Events ---------------------
------------------------------------------------
events = {
    handlers = {}
}

function events.on(event, func)
    if events.handlers[event] == nil then
        events.handlers[event] = {}
    end
    table.insert(events.handlers[event], func)
end

function events.remove_by_prefix(prefix)
    for name, handlers in pairs(events.handlers) do
        local actualname = name
        if type(name) == 'table' then
            actualname = name[1]
        end
        if actualname:sub(1, #prefix+1) == prefix..':' then
            events.handlers[actualname] = nil
        end
    end
end

function pack.unload(prefix)
    events.remove_by_prefix(prefix)
end

function events.emit(event, ...)
    local result = nil
    local handlers = events.handlers[event]
    if handlers == nil then
        return nil
    end
    for _, func in ipairs(handlers) do
        result = result or func(...)
    end
    return result
end

-- class designed for simple UI-nodes access via properties syntax
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

-- the engine automatically creates an instance for every ui document (layout)
Document = {}
function Document.new(docname)
    return setmetatable({name=docname}, {
        __index=function(self, k)
            local elem = Element.new(self.name, k)
            rawset(self, k, elem)
            return elem
        end
    })
end

_GUI_ROOT = Document.new("core:root")
_MENU = _GUI_ROOT.menu
menu = _MENU

local __post_runnables = {}

function __process_post_runnables()
    if #__post_runnables then
        for _, func in ipairs(__post_runnables) do
            func()
        end
        __post_runnables = {}
    end
end

function time.post_runnable(runnable)
    table.insert(__post_runnables, runnable)
end

local log_element = Document.new("core:console").log
function console.log(...)
    local args = {...}
    local text = ''
    for i,v in ipairs(args) do
        if i ~= 1 then 
            text = text..' '..v 
        else
            text = text..v
        end
    end
    log_element.caret = -1
    if log_element.caret > 0 then
        text = '\n'..text
    end
    log_element:paste(text)
end

function gui.template(name, params)
    local text = file.read(file.find("layouts/templates/"..name..".xml"))
    for k,v in pairs(params) do
        local arg = tostring(v):gsub("'", "\\'"):gsub('"', '\\"')
        text = text:gsub("(%%{"..k.."})", arg)
    end
    text = text:gsub("if%s*=%s*'%%{%w+}'", "if=''")
    text = text:gsub("if%s*=%s*\"%%{%w+}\"", "if=\"\"")
    -- remove unsolved properties: attr='%{var}'
    text = text:gsub("%w+%s*=%s*'%%{%w+}'%s?", "")
    text = text:gsub("%w+%s*=%s*\"%%{%w+}\"%s?", "")
    return text
end

session = {
    entries={}
}

function session.get_entry(name)
    local entry = session.entries[name]
    if entry == nil then
        entry = {}
        session.entries[name] = entry
    end
    return entry
end

function session.reset_entry(name)
    session.entries[name] = nil
end

stdcomp = require "core:internal/stdcomp"
entities.get = stdcomp.get_Entity
entities.get_all = function(uids)
    if uids == nil then
        local values = {}
        for k,v in pairs(stdcomp.get_all()) do
            values[k] = v
        end
        return values 
    else
        return stdcomp.get_all(uids)
    end
end

math.randomseed(time.uptime() * 1536227939)

-- --------- Deprecated functions ------ --
local function wrap_deprecated(func, name, alternatives)
    return function (...)
        on_deprecated_call(name, alternatives)
        return func(...)
    end
end

block_index = wrap_deprecated(block.index, "block_index", "block.index")
block_name = wrap_deprecated(block.name, "block_name", "block.name")
blocks_count = wrap_deprecated(block.defs_count, "blocks_count", "block.defs_count")
is_solid_at = wrap_deprecated(block.is_solid_at, "is_solid_at", "block.is_solid_at")
is_replaceable_at = wrap_deprecated(block.is_replaceable_at, "is_replaceable_at", "block.is_replaceable_at")
set_block = wrap_deprecated(block.set, "set_block", "block.set")
get_block = wrap_deprecated(block.get, "get_block", "block.get")
get_block_X = wrap_deprecated(block.get_X, "get_block_X", "block.get_X")
get_block_Y = wrap_deprecated(block.get_Y, "get_block_Y", "block.get_Y")
get_block_Z = wrap_deprecated(block.get_Z, "get_block_Z", "block.get_Z")
get_block_states = wrap_deprecated(block.get_states, "get_block_states", "block.get_states")
set_block_states = wrap_deprecated(block.set_states, "set_block_states", "block.set_states")
get_block_rotation = wrap_deprecated(block.get_rotation, "get_block_rotation", "block.get_rotation")
set_block_rotation = wrap_deprecated(block.set_rotation, "set_block_rotation", "block.set_rotation")
get_block_user_bits = wrap_deprecated(block.get_user_bits, "get_block_user_bits", "block.get_user_bits")
set_block_user_bits = wrap_deprecated(block.set_user_bits, "set_block_user_bits", "block.set_user_bits")

function load_script(path, nocache)
    on_deprecated_call("load_script", "require or loadstring")
    return __load_script(path, nocache)
end

_dofile = dofile
-- Replaces dofile('*/content/packid/*') with load_script('packid:*') 
function dofile(path)
    on_deprecated_call("dofile", "require or loadstring")
    local index = string.find(path, "/content/")
    if index then
        local newpath = string.sub(path, index+9)
        index = string.find(newpath, "/")
        if index then
            local label = string.sub(newpath, 1, index-1)
            newpath = label..':'..string.sub(newpath, index+1)
            if file.isfile(newpath) then
                return __load_script(newpath, true)
            end
        end
    end
    return _dofile(path)
end
