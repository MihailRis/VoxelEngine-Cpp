-- TOML serialization module
local toml = {}

-- Convert table to TOML
function toml.serialize(tb, isinner)
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
            text = text.."["..k.."]\n"..toml.serialize(v).."\n"
        end
    end
    return text
end

-- Parse TOML to new table
function toml.deserialize(s)
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

return toml
