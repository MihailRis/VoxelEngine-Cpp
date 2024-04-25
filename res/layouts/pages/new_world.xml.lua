settings = session.get_entry('new_world')

function world_name_validator(name)
    return name:match("^[%w-\\.\\ ]+$") ~= nil and not world.exists(name)
end

function save_state()
    settings.name = document.name_box.text
    settings.seed = document.seed_box.text
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
    local generator = settings.generator
    session.reset_entry('new_world')
    core.new_world(name, seed, generator)
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
    document.name_box.text = settings.name or ''
    document.seed_box.text = settings.seed or ''
    document.seed_box.placeholder = tostring(math.random()):sub(3)
end
