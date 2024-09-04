# Voxels Chunk (version 2)

IDs and states are separated for extRLE16 compression efficiency.

File format BNF (RFC 5234):

```bnf
chunk    = (65536*uint16)  block ids
           (65536*uint16)  block states

uint16   = 2byte           16 bit little-endian unsigned integer
byte     = %x00-FF         8 bit unsigned integer
```

65536 is number of voxels per chunk (16\*256\*16)

## Block state

Block state is encoded in 16 bits:
- 0-2 bits (3) - block rotation index
- 3-5 bits (3) - segment block bits
- 6-7 bits (2) - reserved
- 8-15 bits (8) - user bits
