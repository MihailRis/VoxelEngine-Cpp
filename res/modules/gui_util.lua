local gui_util = {
    local_dispatchers = {}
}

--- Parse `pagename?arg1=value1&arg2=value2` queries
--- @param query page query string
--- @return page_name, args_table
function gui_util.parse_query(query)
    local args = {}
    local name

    local index = string.find(query, '?')
    if index then
        local argstr = string.sub(query, index + 1)
        name = string.sub(query, 1, index - 1)
        
        for key, value in string.gmatch(argstr, "([^=&]*)=([^&]*)") do
            args[key] = value
        end
    else
        name = query
    end
    return name, args
end

--- @param query page query string
--- @return document_id
function gui_util.load_page(query)
    local name, args = gui_util.parse_query(query)
    for i = #gui_util.local_dispatchers, 1, -1 do
        local newname, newargs = gui_util.local_dispatchers[i](name, args)
        name = newname or name
        args = newargs or args
    end
    local filename = file.find(string.format("layouts/pages/%s.xml", name))
    if filename then
        name = file.prefix(filename)..":pages/"..name
        gui.load_document(filename, name, args)
        return name
    end
end

function gui_util.add_page_dispatcher(dispatcher)
    if type(dispatcher) ~= "function" then
        error("function expected")
    end
    table.insert(gui_util.local_dispatchers, dispatcher)
end

function gui_util.reset_local()
    gui_util.local_dispatchers = {}
end

return gui_util
