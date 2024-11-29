local base_entities = {}

function base_entities.drop(ppos, itemid, count, pickup_delay)
    if itemid == 0 or not itemid then
        return nil
    end
    return entities.spawn("base:drop", ppos, {base__drop={
        id=itemid,
        count=count,
        pickup_delay=pickup_delay
    }})
end

return base_entities
