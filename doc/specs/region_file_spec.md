# Region File (version 3)

File format BNF (RFC 5234):

```bnf
file    = header (*chunk) offsets   complete file
header  = magic %x02 byte           magic number, version and compression
                                    method

magic   = %x2E %x56 %x4F %x58       '.VOXREG\0'
          %x52 %x45 %x47 %x00

chunk   = uint32 uint32 (*byte)     byte array with size and source size 
                                    prefix where source size is 
                                    decompressed chunk data size

offsets = (1024*uint32)             offsets table
int32   = 4byte                     unsigned big-endian 32 bit integer
byte    = %x00-FF                   8 bit unsigned integer
```

C struct visualization:

```c
typedef unsigned char byte;

struct file {
	// 10 bytes
	struct {
		char magic[8] = ".VOXREG";
		byte version = 3;
		byte compression;
	} header;
	
	struct {
		uint32_t size; // byteorder: little-endian
		uint32_t sourceSize; // byteorder: little-endian
		byte* data;
	} chunks[1024]; // file does not contain zero sizes for missing chunks
	
	uint32_t offsets[1024]; // byteorder: little-endian
};
```

Offsets table contains chunks positions in file. 0 means that chunk is not present in the file. Minimal valid offset is 10 (header size).

Available compression methods:
0. no compression
1. extRLE8
2. extRLE16
