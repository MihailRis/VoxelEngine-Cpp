settings = session.get_entry('new_world')

function world_name_validator(name)
    return name:match("^[%w-\\.\\ ]+$") ~= nil and not world.exists(name)
end

function settings.generator_name(id)
    local prefix, name = parse_path(id)
    if prefix == "core" then
        return gui.str(name, "world.generators")
    else
        return id
    end
end

function create_world()
    if not document.name_box.valid then
        return
    end
    local name = document.name_box.text
    local seed = document.seed_box.text
    core.new_world(name, seed, settings.generator)
end

function on_open()
    if settings.generator == nil then
        settings.generator = core.get_default_generator()
    end
    document.generator_btn.text = string.format(
        "%s: %s", 
        gui.str("World generator", "world"), 
        settings.generator_name(settings.generator)
    )
    document.seed_box.placeholder = tostring(math.random()):sub(3)
end
