function on_open(report)
    for i, entry in ipairs(report.content) do
        document.content_panel:add(gui.template("content_entry", entry))
    end
end
