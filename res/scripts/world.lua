-- use for engine development tests
-- must be empty in release
-- must not be modified by content-packs

local server = network.tcp_open(65343, function (socket)
    print("connected client", socket.id)
    socket:send(utf8.tobytes("Hello, World!"))
    socket:close()
end)
print("server", server.id)

local socket = network.tcp_connect('localhost', 65343, function (socket)
    print("connected", socket.id)
end)

function on_world_tick()
    local result = socket:recv(128)
    if result and #result > 0 then
        print("received from server:", utf8.tostring(result)) 
    end
end

function on_world_quit()
    server:close()
end
