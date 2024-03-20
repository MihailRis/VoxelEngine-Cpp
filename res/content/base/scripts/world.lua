local stream_id

function on_world_open()
	stream_id = audio.play_stream_2d("sounds/musics/", 1, 1, "music", true)
	print("All ok")
end

function on_world_quit()
    audio.stop(stream_id)
end
