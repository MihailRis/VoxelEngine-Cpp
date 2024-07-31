function table.Copy(t)
    local newT = {}
    for k, v in pairs(t) do
        newT[k] = v
    end
    return newT
end

function table.Random(t)
    local randomT = {}
    for k, v in pairs(t) do randomT[#randomT + 1] = {k, v} end

    local var = randomT[math.random(1, #randomT)]
    local key, value = var[1], var[2]

    return value, key
end