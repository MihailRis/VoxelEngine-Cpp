function run_script(path)
    debug.log("starting application script "..path)

    local code = file.read(path)
    local chunk, err = loadstring(code, path)
    if chunk == nil then
        error(err)
    end
    setfenv(chunk, setmetatable({app=__vc_app}, {__index=_G}))
    start_coroutine(chunk, path)
end

function refresh()
    document.list:clear()
    
    local available = pack.get_available()
    local infos = pack.get_info(available)
    for _, name in ipairs(available) do
        local info = infos[name]
        local scripts_dir = info.path.."/scripts/app"
        if not file.exists(scripts_dir) then
            goto continue
        end
        local files = file.list(scripts_dir)
        for _, filename in ipairs(files) do
            if file.ext(filename) == "lua" then
                document.list:add(gui.template("script", {
                    pack=name,
                    name=file.stem(filename),
                    path=filename
                }))
            end
        end
        ::continue::
    end
end

function on_open()
    refresh()
end
