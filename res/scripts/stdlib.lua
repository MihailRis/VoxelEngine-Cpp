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

local __cached_scripts = {}
local __cached_results = {}

-- Load script with caching
--
-- path - script path `contentpack:filename`. 
--     Example `base:scripts/tests.lua`
--
-- nocache - ignore cached script, load anyway
function load_script(path, nocache)
    local packname, filename = parse_path(path)
    local fullpath = file.resolve(path);

    -- __cached_scripts used in condition because cached result may be nil
    if not nocache and __cached_scripts[fullpath] ~= nil then
        return __cached_results[fullpath]
    end
    if not file.isfile(path) then
        error("script '"..filename.."' not found in '"..packname.."'")
    end

    local script, err = loadfile(fullpath)
    if script == nil then
        error(err)
    end
    local result = script()
    if not nocache then
        __cached_scripts[fullpath] = script
        __cached_results[fullpath] = result
    end
    return result
end

function require(path)
    local prefix, file = parse_path(path)
    return load_script(prefix..":modules/"..file..".lua")
end

function __reset_scripts_cache()
    __cached_scripts = {}
    __cached_results = {}
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
