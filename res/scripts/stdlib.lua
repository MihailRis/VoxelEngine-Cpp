-- kit of standard functions

-- Check if given table is an array
function is_array(x)
    if #t > 0 then
        return true
    end
    for k, v in pairs(x) do
        return false
    end
    return true
end

-- Get entry-point and filename from `entry-point:filename` path 
function parse_path(path)
    local index = string.find(path, ':')
    if index == nil then
        error("invalid path syntax (':' missing)")
    end
    return string.sub(path, 1, index-1), string.sub(path, index+1, -1)
end

package = {
    loaded={}
}
local __cached_scripts = {}

-- Load script with caching
--
-- path - script path `contentpack:filename`. 
--     Example `base:scripts/tests.lua`
--
-- nocache - ignore cached script, load anyway
function load_script(path, nocache)
    local packname, filename = parse_path(path)

    -- __cached_scripts used in condition because cached result may be nil
    if not nocache and __cached_scripts[path] ~= nil then
        return package.loaded[path]
    end
    if not file.isfile(path) then
        error("script '"..filename.."' not found in '"..packname.."'")
    end

    local script, err = load(file.read(path), path)
    if script == nil then
        error(err)
    end
    local result = script()
    if not nocache then
        __cached_scripts[path] = script
        package.loaded[path] = result
    end
    return result
end

function __scripts_cleanup()
    print("cleaning scripts cache")
    for k, v in pairs(__cached_scripts) do
        local packname, _ = parse_path(k)
        if packname ~= "core" then
            print("unloaded "..k)
            __cached_scripts[k] = nil
            package.loaded[k] = nil
        end
    end
end

function require(path)
    local prefix, file = parse_path(path)
    return load_script(prefix..":modules/"..file..".lua")
end

function sleep(timesec)
    local start = time.uptime()
    while time.uptime() - start < timesec do
        coroutine.yield()
    end
end

_dofile = dofile
-- Replaces dofile('*/content/packid/*') with load_script('packid:*') 
function dofile(path)
    local index = string.find(path, "/content/")
    if index then
        local newpath = string.sub(path, index+9)
        index = string.find(newpath, "/")
        if index then
            local label = string.sub(newpath, 1, index-1)
            newpath = label..':'..string.sub(newpath, index+1)
            if file.isfile(newpath) then
                return load_script(newpath, true)
            end
        end
    end
    return _dofile(path)
end

function pack.is_installed(packid)
    return file.isfile(packid..":package.json")
end

function pack.data_file(packid, name)
    file.mkdirs("world:data/"..packid)
    return "world:data/"..packid.."/"..name
end

vec2_mt = {}
function vec2_mt.__tostring(self)
    return "vec2("..self[1]..", "..self[2]..")"
end

vec3_mt = {}
function vec3_mt.__tostring(self)
    return "vec3("..self[1]..", "..self[2]..", "..self[3]..")"
end

vec4_mt = {}
function vec4_mt.__tostring(self)
    return "vec4("..self[1]..", "..self[2]..", "..self[3]..", "..self[4]..")"
end

color_mt = {}
function color_mt.__tostring(self)
    return "rgba("..self[1]..", "..self[2]..", "..self[3]..", "..self[4]..")"
end

-- events
events = {
    handlers = {}
}

function events.on(event, func)
    events.handlers[event] = events.handlers[event] or {}
    table.insert(events.handlers[event], func)
end

function events.remove_by_prefix(prefix)
    for name, handlers in pairs(events.handlers) do
        if name:sub(1, #prefix) == prefix then
            events.handlers[name] = nil
        end
    end
end

function pack.unload(prefix)
    events.remove_by_prefix(prefix)
end

function events.emit(event, ...)
    result = nil
    if events.handlers[event] then
        for _, func in ipairs(events.handlers[event]) do
            result = result or func(...)
        end
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
            return Element.new(self.name, k)
        end
    })
end

-- Deprecated functions
block_index = block.index
block_name = block.name
blocks_count = block.defs_count
is_solid_at = block.is_solid_at
is_replaceable_at = block.is_replaceable_at
set_block = block.set
get_block = block.get
get_block_X = block.get_X
get_block_Y = block.get_Y
get_block_Z = block.get_Z
get_block_states = block.get_states
set_block_states = block.set_states
get_block_rotation = block.get_rotation
set_block_rotation = block.set_rotation
get_block_user_bits = block.get_user_bits
set_block_user_bits = block.set_user_bits
