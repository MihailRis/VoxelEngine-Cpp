# Fragment file (.vox)

Current version is 1.

Fragment file is [vcbjson file](binary_json_spec.md).

JSON schema representation format in [Orderly](https://orderly-json.org/docs/) language:

```
object {
    integer version {1,};
    array {integer;} {1,} size;
    array {string;} block-names;
    array {integer;} {0,65535} voxels;
}
```

Where:
- **version** is fragment file format version.
- **size** is fragment size (three positive integers).
- **block-names** is array of full block names: `pack_id:block_name`.
- **voxels**: voxels data array: *index, state, index, state...*
  where:
  - index is based on block-names array.
  - state - see [block state](region_voxels_chunk_spec.md#block-state).

**block-names** must have `core:air` as the first element.
