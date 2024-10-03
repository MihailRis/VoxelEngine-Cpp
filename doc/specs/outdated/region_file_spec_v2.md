# Region File (version 2)

File format BNF (RFC 5234):

```bnf
file    = header (*chunk) offsets   complete file
header  = magic %x02 %x00           magic number, version and reserved
                                    zero byte

magic   = %x2E %x56 %x4F %x58       '.VOXREG\0'
          %x52 %x45 %x47 %x00

chunk   = int32 (*byte)             byte array with size prefix
offsets = (1024*int32)              offsets table
int32   = 4byte                     signed big-endian 32 bit integer
byte    = %x00-FF                   8 bit unsigned integer
```

C struct visualization:

```c
typedef unsigned char byte;

struct file {
	// 10 bytes
	struct {
		char magic[8] = ".VOXREG";
		byte version = 2;
		byte reserved = 0;
	} header;
	
	struct {
		int32_t size; // byteorder: big-endian
		byte* data;
	} chunks[1024]; // file does not contain zero sizes for missing chunks
	
	int32_t offsets[1024]; // byteorder: big-endian
};
```

Offsets table contains chunks positions in file. 0 means that chunk is not present in the file. Minimal valid offset is 10 (header size).
