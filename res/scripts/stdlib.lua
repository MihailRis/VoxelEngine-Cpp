------------------------------------------------
------ Extended kit of standard functions ------
------------------------------------------------

function sleep(timesec)
    local start = time.uptime()
    while time.uptime() - start < timesec do
        coroutine.yield()
    end
end

function tb_frame_tostring(frame)
    local s = frame.short_src
    if frame.what ~= "C" then
        s = s .. ":" .. tostring(frame.currentline)
    end
    if frame.what == "main" then
        s = s .. ": in main chunk"
    elseif frame.name then
        s = s .. ": in function " .. utf8.escape(frame.name)
    end
    return s
end

if test then
    test.sleep = sleep
    test.name = __VC_TEST_NAME
    test.new_world = core.new_world
    test.open_world = core.open_world
    test.close_world = core.close_world
    test.reopen_world = core.reopen_world
    test.delete_world = core.delete_world
    test.reconfig_packs = core.reconfig_packs
    test.set_setting = core.set_setting
    test.tick = coroutine.yield

    function test.quit()
        local tb = debug.get_traceback(1)
        local s = "test.quit() traceback:"
        for i, frame in ipairs(tb) do
            s = s .. "\n\t"..tb_frame_tostring(frame)
        end
        debug.log(s)
        core.quit()
        coroutine.yield()
    end

    function test.sleep_until(predicate, max_ticks)
        max_ticks = max_ticks or 1e9
        local ticks = 0
        while ticks < max_ticks and not predicate() do
            test.tick()
        end
        if ticks == max_ticks then
            error("max ticks exceed")
        end
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

function events.reset(event, func)
    if func == nil then
        events.handlers[event] = nil
    else
        events.handlers[event] = {func}
    end
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

local _RadioGroup = {}
function _RadioGroup.set(self, key)
    if type(self) ~= 'table' then
        error("called as non-OOP via '.', use radiogroup:set")
    end
    if self.current then
        self.elements[self.current].enabled = true
    end
    self.elements[key].enabled = false
    self.current = key
    if self.callback then
        self.callback(key)
    end
end
function _RadioGroup.__call(self, elements, onset, default)
    local group = setmetatable({
        elements=elements, 
        callback=onset, 
        current=nil
    }, {__index=_RadioGroup})
    group:set(default)
    return group
end
setmetatable(_RadioGroup, _RadioGroup)
RadioGroup = _RadioGroup

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

---  Console library extension ---
console.cheats = {}

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

rules = {nexid = 1, rules = {}}
local _rules = rules

function _rules.get_rule(name)
    local rule = _rules.rules[name]
    if rule == nil then
        rule = {listeners={}}
        _rules.rules[name] = rule
    end
    return rule
end

function _rules.get(name)
    local rule = _rules.rules[name]
    if rule == nil then
        return nil
    end
    return rule.value
end

function _rules.set(name, value)
    local rule = _rules.get_rule(name)
    rule.value = value
    for _, handler in pairs(rule.listeners) do
        handler(value)
    end
end

function _rules.reset(name)
    local rule = _rules.get_rule(name)
    _rules.set(rule.default)
end

function _rules.listen(name, handler)
    local rule = _rules.get_rule(name)
    local id = _rules.nexid
    _rules.nextid = _rules.nexid + 1
    rule.listeners[utf8.encode(id)] = handler
    return id
end

function _rules.create(name, value, handler)
    local rule = _rules.get_rule(name)
    rule.default = value

    local handlerid
    if handler ~= nil then
        handlerid = _rules.listen(name, handler)
    end
    if _rules.get(name) == nil then
        _rules.set(name, value)
    elseif handler then
        handler(_rules.get(name))
    end
    return handlerid
end

function _rules.unlisten(name, id)
    local rule = _rules.rules[name]
    if rule == nil then
        return
    end
    rule.listeners[utf8.encode(id)] = nil
end

function _rules.clear()
    _rules.rules = {}
    _rules.nextid = 1
end

function __vc_on_hud_open()
    _rules.create("allow-cheats", true)

    _rules.create("allow-content-access", hud._is_content_access(), function(value)
        hud._set_content_access(value)
    end)
    _rules.create("allow-flight", true, function(value)
        input.set_enabled("player.flight", value)
    end)
    _rules.create("allow-noclip", true, function(value)
        input.set_enabled("player.noclip", value)
    end)
    _rules.create("allow-attack", true, function(value)
        input.set_enabled("player.attack", value)
    end)
    _rules.create("allow-destroy", true, function(value)
        input.set_enabled("player.destroy", value)
    end)
    _rules.create("allow-cheat-movement", true, function(value)
        input.set_enabled("movement.cheat", value)
    end)
    _rules.create("allow-fast-interaction", true, function(value)
        input.set_enabled("player.fast_interaction", value)
    end)
    _rules.create("allow-debug-cheats", true, function(value)
        hud._set_debug_cheats(value)
    end)
    input.add_callback("devtools.console", function()
        if hud.is_paused() then
            return
        end
        time.post_runnable(function()
            hud.show_overlay("core:console", false, {"console"})
        end)
    end)
end

local RULES_FILE = "world:rules.toml"
function __vc_on_world_open()
    if not file.exists(RULES_FILE) then
        return
    end
    local rule_values = toml.parse(file.read(RULES_FILE))
    for name, value in pairs(rule_values) do
        _rules.set(name, value)
    end
end

function __vc_on_world_save()
    local rule_values = {}
    for name, rule in pairs(rules.rules) do
        rule_values[name] = rule.value
    end
    file.write(RULES_FILE, toml.tostring(rule_values))
end

function __vc_on_world_quit()
    _rules.clear()
end

local __vc_coroutines = {}
local __vc_next_coroutine = 1
local __vc_coroutine_error = nil

function __vc_start_coroutine(chunk)
    local co = coroutine.create(function()
        local status, err = pcall(chunk)
        if not status then
            __vc_coroutine_error = err
        end
    end)
    local id = __vc_next_coroutine
    __vc_next_coroutine = __vc_next_coroutine + 1
    __vc_coroutines[id] = co
    return id
end

function __vc_resume_coroutine(id)
    local co = __vc_coroutines[id]
    if co then
        coroutine.resume(co)
        if __vc_coroutine_error then
            error(__vc_coroutine_error)
        end
        return coroutine.status(co) ~= "dead"
    end
    return false
end

function __vc_stop_coroutine(id)
    local co = __vc_coroutines[id]
    if co then
        if coroutine.close then
            coroutine.close(co)
        end
        __vc_coroutines[id] = nil
    end
end

assets = {}
assets.load_texture = core.__load_texture

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
