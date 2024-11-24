local base_entities = {}

function base_entities.drop(ppos, itemid, count, ready)
    return entities.spawn("base:drop", ppos, {base__drop={
        id=itemid,
        count=count,
        ready=ready
    }})
end

return base_entities
