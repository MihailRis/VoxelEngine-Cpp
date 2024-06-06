print("WARNING: toml is replaced with built-in library, just remove 'require \"core:toml\"'")
toml.serialize = toml.tostring
toml.deserialize = toml.parse
return toml
