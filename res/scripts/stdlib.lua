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
    local script = loadfile(fullpath)
    if script == nil then
        error("script '"..filename.."' not found in '"..packname.."'")
    end
    local result = script()
    if not nocache then
        __cached_scripts[fullpath] = script
        __cached_results[fullpath] = result
    end
    return result
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

toml = {}

-- Convert table to TOML
function toml.from_table(tb, isinner)
    local text = ""
    for k, v in pairs(tb) do
        local tp = type(v)
        if tp ~= "table" then
            text = text..k.." = "
            if tp == "string" then
                text = text..string.format("%q", v)
            else
                text = text..tostring(v)
            end
            text = text.."\n"
        end
    end
    for k, v in pairs(tb) do
        local tp = type(v)
        if tp == "table" then
            if isinner then
                error("only one level of subtables supported")
            end
            text = text.."["..k.."]\n"..toml.from_table(v).."\n"
        end
    end
    return text
end

-- Parse TOML to new table
function toml.parse(s)
    local output = {}
    local current = output
    local lines = {}
    for line in string.gmatch(s, "[^\r\n]+") do
        line = string.gsub(line, "%s+", "")
        table.insert(lines, line)
    end
    for i = 1,#lines do
        local s = lines[i]
        if string.sub(s, 1, 1) == "[" then
            local section = s.sub(s, 2, #s-1)
            current = {}
            output[section] = current
        else 
            for k, v in string.gmatch(s, "(%w+)=(.+)" ) do
                v = string.gsub(v, "%s+", "")
                if v.sub(v, 1, 1) == "\"" then
                    current[k] = v.sub(v, 2, #v-1)
                elseif v == "true" or v == "false" then
                    current[k] = v == "true"
                end
                
                local num = tonumber(v)
                if num ~= nil then
                    current[k] = num
                end
            end
        end
    end
    return output
end
