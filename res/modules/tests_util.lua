local util = {}

function util.create_demo_world(generator)
    app.config_packs({"base"})
    app.new_world("demo", "2019", generator or "core:default")
end

return util
