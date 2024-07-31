local hook = {}

hook.list = {}

function hook.Add(id, name, fn)
    if not hook.list[id] then hook.list[id] = {} end

    table.insert(hook.list[id], {
        name = name,
        fn = fn
    })
end


function hook.Remove(id, name)
    if not hook.list[id] then return end

    for k, v in pairs(hook.list[id]) do
        if v.name == name then
            table.remove(hook.list[id], k)
            break
        end
    end
end

function hook.Run(id, ...)
    if not hook.list[id] then return end

    local result = nil

    for i = 1, #hook.list[id] do
        local fn = hook.list[id][i].fn

        local fnRes = fn(...)
        result = fnRes or result
    end

    return result
end

return hook