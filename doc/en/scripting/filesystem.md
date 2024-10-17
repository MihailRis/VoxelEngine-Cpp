# Filesystem and serialization

### *file* library

Filesystem interaction library.

```python
file.resolve(path: str) -> str
```

Function turns `entry_point:path` (example `user:worlds/house1`) to a regular path. (example `C://Users/user/.voxeng/worlds/house1`)

> [!NOTE]
> The function should be used for debug only. *entry_point:path* notation is required in all **file** functions.

Resulting path is not canonical and may be relative.

```python
file.read(path: str) -> str
```

Read whole text file.

```python
file.read_bytes(path: str) -> array of integers
```

Read file into bytes array.

```python
file.write(path: str, text: str) -> nil
```

Overwrite text file.

```python
file.write_bytes(path: str, data: array of integers)
```

Overwrite binary file with bytes array.

```python
file.length(path: str) -> int
```

Get file length (bytes) or 0.

```python
file.exists(path: str) -> bool
```

Check if file or directory exist.

```python
file.isfile(path: str) -> bool
```

Check if the path points to a file.

```python
file.isdir(path: str) -> bool
```

Check if the path points to a directory.

```python
file.mkdir(path: str) -> bool
```

Create directory. Returns true if new directory created

```python
file.mkdirs(path: str) -> bool
```

Create directories chain. Returns true if new directory created.

## *json* library

The library contains functions for serializing and deserializing tables:

```python
json.tostring(object: table, human_readable: bool=false) -> str
```

Serializes an object into a JSON string. If the second parameter is **true**, multi-line human-readable formatting will be used, rather than the compact format used by default.

```python
json.parse(code: str) -> table
```

Parses a JSON string into a table.

## *toml* library

The library contains functions for serializing and deserializing tables:

```python
toml.tostring(object: table) -> str
```

Serializes an object into a TOML string.

```python
toml.parse(code: str) -> table
```

Parses a TOML string into a table.

## *bjson* library

The library contains functions for working with the binary data exchange format [vcbjson](../../specs/binary_json_spec.md).

```lua
-- Encodes a table into a byte array
bjson.tobytes(
    -- encoded table
    value: table,
    -- compression
    [optional] compression: bool=true
) --> Bytearray

-- Decodes a byte array into a table
bjson.frombytes(bytes: table | Bytearray) --> table
```

## Storing data in a world

When saving pack data in the world, you should use the function:

```python
pack.data_file(packid: str, filename: str) -> str
```

Returns data file path like: `world:data/packid/filename`
and creates missing directories.

If paths other than `data/{packid}/...` are used, data may be lost.
