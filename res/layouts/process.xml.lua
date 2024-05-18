function on_progress(done, total)
    local progress = done / total
    document.progress_label.text = string.format(
        "%s/%s (%s%%)", done, total, math.floor(progress*100)
    )
end

function on_open(title)
    document.title_label.text = title
end
