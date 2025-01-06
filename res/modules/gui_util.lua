local gui_util = {}

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
        
        local map = {}
        for key, value in string.gmatch(argstr, "([^=&]*)=([^&]*)") do
            map[key] = value
        end
        table.insert(args, map)
    else
        name = query
    end
    return name, args
end

--- @param query page query string
--- @return document_id
function gui_util.load_page(query)
    local name, args = gui_util.parse_query(query)
    local filename = file.find(string.format("layouts/pages/%s.xml", name))
    if filename then
        name = file.prefix(filename)..":pages/"..name
        gui.load_document(filename, name, args)
        return name
    end
end

return gui_util
