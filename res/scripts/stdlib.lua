-- kit of standard functions

-- Check if given table is an array
function is_array(t)
    if #t > 0 then
        return true
    end
    for k, v in pairs(t) do
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
local __warnings_hidden = {}

function on_deprecated_call(name, alternatives)
    if __warnings_hidden[name] then
        return
    end
    __warnings_hidden[name] = true
    if alternatives then
        debug.warning("deprecated function called ("..name.."), use "..
            alternatives.." instead\n"..debug.traceback())
    else
        debug.warning("deprecated function called ("..name..")\n"..debug.traceback())
    end
end

-- Load script with caching
--
-- path - script path `contentpack:filename`. 
--     Example `base:scripts/tests.lua`
--
-- nocache - ignore cached script, load anyway
local function __load_script(path, nocache)
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
    return __load_script(prefix..":modules/"..file..".lua")
end

function sleep(timesec)
    local start = time.uptime()
    while time.uptime() - start < timesec do
        coroutine.yield()
    end
end

function pack.is_installed(packid)
    return file.isfile(packid..":package.json")
end

function pack.data_file(packid, name)
    file.mkdirs("world:data/"..packid)
    return "world:data/"..packid.."/"..name
end

-- events
events = {
    handlers = {}
}

function events.on(event, func)
    -- why an array? length is always = 1
    -- FIXME: temporary fixed
    events.handlers[event] = {} -- events.handlers[event] or {}
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

function timeit(iters, func, ...)
    local tm = time.uptime()
    for i=1,iters do
        func(...)
    end
    print("[time mcs]", (time.uptime()-tm) * 1000000)
end

function table.has(t, x)
    for i,v in ipairs(t) do
        if v == x then
            return true
        end
    end
    return false
end

function table.index(t, x)
    for i,v in ipairs(t) do
        if v == x then
            return i
        end
    end
    return -1
end

function table.remove_value(t, x)
    local index = table.index(t, x)
    if index ~= -1 then
        table.remove(t, index)
    end
end

function table.tostring(t)
    local s = '['
    for i,v in ipairs(t) do
        s = s..tostring(v)
        if i < #t then
            s = s..', '
        end
    end
    return s..']'
end

function file.readlines(path)
    local str = file.read(path)
    local lines = {}
    for s in str:gmatch("[^\r\n]+") do
        table.insert(lines, s)
    end
    return lines
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

----------------------------------------------

function math.clamp(_in, low, high)
    return math.min(math.max(_in, low), high)
end

function math.rand(low, high)
    return low + (high - low) * math.random()
end

----------------------------------------------

function table.copy(t)
    local copied = {}

    for k, v in pairs(t) do
        copied[k] = v
    end

    return copied
end

function table.count_pairs(t)
    local count = 0

    for k, v in pairs(t) do
        count = count + 1
    end

    return count
end

function table.random(t)
    return t[math.random(1, #t)]
end

----------------------------------------------

local pattern_escape_replacements = {
    ["("] = "%(",
    [")"] = "%)",
    ["."] = "%.",
    ["%"] = "%%",
    ["+"] = "%+",
    ["-"] = "%-",
    ["*"] = "%*",
    ["?"] = "%?",
    ["["] = "%[",
    ["]"] = "%]",
    ["^"] = "%^",
    ["$"] = "%$",
    ["\0"] = "%z"
}

function string.pattern_safe(str)
    return string.gsub(str, ".", pattern_escape_replacements)
end

--local totable = string.ToTable
local string_sub = string.sub
local string_find = string.find
local string_len = string.len
function string.explode(separator, str, withpattern)
    --if (separator == "") then return totable(str) end
    if (withpattern == nil) then withpattern = false end

    local ret = {}
    local current_pos = 1

    for i = 1, string_len(str) do
        local start_pos, end_pos = string_find(str, separator, current_pos, not withpattern)
        if (not start_pos) then break end
        ret[i] = string_sub(str, current_pos, start_pos - 1)
        current_pos = end_pos + 1
    end

    ret[#ret + 1] = string_sub(str, current_pos)

    return ret
end

function string.split(str, delimiter)
    return string.explode(delimiter, str)
end

function string.formatted_time(seconds, format)
    if (not seconds) then seconds = 0 end
    local hours = math.floor(seconds / 3600)
    local minutes = math.floor((seconds / 60) % 60)
    local millisecs = (seconds - math.floor(seconds)) * 1000
    seconds = math.floor(seconds % 60)

    if (format) then
        return string.format(format, minutes, seconds, millisecs)
    else
        return { h = hours, m = minutes, s = seconds, ms = millisecs }
    end
end

function string.replace(str, tofind, toreplace)
    local tbl = string.Explode(tofind, str)
    if (tbl[1]) then return table.concat(tbl, toreplace) end
    return str
end

function string.trim(s, char)
    if char then char = string.pattern_safe(char) else char = "%s" end
    return string.match(s, "^" .. char .. "*(.-)" .. char .. "*$") or s
end

function string.trim_right(s, char)
    if char then char = string.pattern_safe(char) else char = "%s" end
    return string.match(s, "^(.-)" .. char .. "*$") or s
end

function string.trim_left(s, char)
    if char then char = string.pattern_safe(char) else char = "%s" end
    return string.match(s, "^" .. char .. "*(.+)$") or s
end

local meta = getmetatable("")

function meta:__index(key)
    local val = string[key]
    if (val ~= nil) then
        return val
    elseif (tonumber(key)) then
        return string.sub(self, key, key)
    end
end

function meta:__len()
    return string.len(self) -- budet herovo rabotat s utf8, ili kirillitsa
end

function string.starts_with(str, start)
    return string.sub(str, 1, string.len(start)) == start
end

function string.ends_with(str, endStr)
    return endStr == "" or string.sub(str, -string.len(endStr)) == endStr
end

function string.unpack(...)
    local t = {...}
    t = type(t[1]) == "table" and t[1] or t

    local str = ""

    for k, v in pairs(t) do
        str = str .. tostring(v)
    end

    return str
end

----------------------------------------------

hook = require("core:internal/hook")

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
