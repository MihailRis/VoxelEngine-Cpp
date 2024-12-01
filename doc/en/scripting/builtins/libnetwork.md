# *network* library

A library for working with the network.

## HTTP requests

```lua
-- Performs a GET request to the specified URL.
-- After receiving the response, passes the text to the callback function.
network.get(url: str, callback: function(str))

-- Example:
network.get("https://api.github.com/repos/MihailRis/VoxelEngine-Cpp/releases/latest", function (s)
    print(json.parse(s).name) -- will output the name of the latest engine release
end)

-- A variant for binary files, with a byte array instead of a string in the response.
network.get_binary(url: str, callback: function(table|ByteArray))
```

## TCP Connections

```lua
network.tcp_connect(
    -- Address
    address: str,
    -- Port
    port: int,
    -- Function called upon successful connection
    -- Sending will not work before connection
    -- Socket is passed as the only argument
    callback: function(Socket)
) --> Socket
```

Initiates TCP connection.

The Socket class has the following methods:

```lua
-- Sends a byte array
socket:send(table|ByteArray)

-- Reads the received data
socket:recv(
    -- Maximum size of the byte array to read
    length: int,
    -- Use table instead of Bytearray
    [optional] usetable: bool=false
) -> nil|table|Bytearray
-- Returns nil on error (socket is closed or does not exist).
-- If there is no data yet, returns an empty byte array.

-- Closes the connection
socket:close()

-- Checks that the socket exists and is not closed.
socket:is_alive() --> bool

-- Checks if the connection is present (using socket:send(...) is available).
socket:is_connected() --> bool

-- Returns the address and port of the connection.
socket:get_address() --> str, int
```

```lua
-- Opens a TCP server.
network.tcp_open(
    -- Port
    port: int,
    -- Function called when connecting
    -- The socket of the connected client is passed as the only argument
    callback: function(Socket)
) --> ServerSocket
```

The SocketServer class has the following methods:

```lua
-- Closes the server, breaking connections with clients.
server:close()

-- Checks if the TCP server exists and is open.
server:is_open() --> bool

-- Returns the server port.
server:get_port() --> int
```

## Analytics

```lua
-- Returns the approximate amount of data sent (including connections to localhost)
-- in bytes.
network.get_total_upload() --> int
-- Returns the approximate amount of data received (including connections to localhost)
-- in bytes.
network.get_total_download() --> int
```
