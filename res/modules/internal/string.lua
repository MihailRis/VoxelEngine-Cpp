function string.FormattedTime(seconds, format ) -- from gmod
    if not seconds then seconds = 0 end

    local hours = math.floor(seconds / 3600)
    local minutes = math.floor((seconds / 60) % 60)
    local millisecs = (seconds - math.floor(seconds)) * 100
    seconds = math.floor(seconds % 60)

    if format then
        return string.format(format, minutes, seconds, millisecs)
    else
        return {h = hours, m = minutes, s = seconds, ms = millisecs}
    end
end

function string.Left(str, num) return string.sub(str, 1, num) end
function string.Right(str, num) return string.sub(str, -num) end

function string.Replace(str, tofind, toreplace)
    local tbl = string.Explode(tofind, str)
    if (tbl[1]) then return table.concat(tbl, toreplace) end
    return str
end

local totable = string.ToTable
local string_sub = string.sub
local string_find = string.find
local string_len = string.len
function string.Explode(separator, str, withpattern)
    if (separator == "") then return totable(str) end
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

function string.Split( str, delimiter )
    return string.Explode( delimiter, str )
end

function string.Trim(s, char)
    if char then char = string.PatternSafe(char) else char = "%s" end
    return string.match(s, "^" .. char .. "*(.-)" .. char .. "*$") or s
end

function string.TrimRight(s, char)
    if char then char = string.PatternSafe(char) else char = "%s" end
    return string.match(s, "^(.-)" .. char .. "*$") or s
end

function string.TrimLeft(s, char)
    if char then char = string.PatternSafe(char) else char = "%s" end
    return string.match(s, "^" .. char .. "*(.+)$") or s
end