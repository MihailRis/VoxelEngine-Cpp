local base_entities = {}

function base_entities.drop(ppos, itemid, count)
    return entities.spawn("base:drop", ppos, {base__drop={
        id=itemid,
        count=count
    }})
end

return base_entities
