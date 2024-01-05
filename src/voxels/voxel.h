#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"

const u_int32 BLOCK_DIR_NORTH = 0x0;
const u_int32 BLOCK_DIR_WEST = 0x1;
const u_int32 BLOCK_DIR_SOUTH = 0x2;
const u_int32 BLOCK_DIR_EAST = 0x3;
const u_int32 BLOCK_DIR_UP = 0x4;
const u_int32 BLOCK_DIR_DOWN = 0x5;


const u_int32 BLOCK_DIR_MASK       = 0b111;
const u_int32 BLOCK_DIR_OFFSET     = 0x0;

const u_int32 BLOCK_SIG_MASK       = 0b1;
const u_int32 BLOCK_SIG_OFFSET     = 0x3;

const u_int32 BLOCK_VARIANT_MASK   = 0b1111;
const u_int32 BLOCK_VARIANT_OFFSET = 0x4;

const u_int32 BLOCK_BITS_MASK      = 0b11111111;
const u_int32 BLOCK_BITS_OFFSET    = 0x8;
const u_int32   BLOCK_VALUE_MASK   = 0b1111;
const u_int32   BLOCK_VALUE_OFFSET = 0x8;
const u_int32   BLOCK_STATE_MASK   = 0b1111;
const u_int32   BLOCK_STATE_OFFSET = 0xC;

//*-----------------------------------------------------------------------|
//*                                   16    19 20     24      28      32  |
//*   |                               |     | /       |       |       |   |
//*   |* * * * * * * * * * * * * * * *|* * *|*|* * * *|* * * *|*'*'*'*|   |
//*                   id                dir / \ variant  value  4states   |
//*                                        |sig|                          |
//*-----------------------------------------------------------------------|

struct voxel {
	blockid_t id;
	u_short16 states;

	inline u_int32 getId()  const {
		return id;
	}

	inline u_char8 getDir() const {
		return (states >> BLOCK_DIR_OFFSET) & BLOCK_DIR_MASK;
	}

	inline void setDir(u_char8 dir) {
		states &= ~BLOCK_DIR_MASK << BLOCK_DIR_OFFSET;
		states |= dir << BLOCK_DIR_OFFSET;
	}

	inline u_char8 getVariant() const {
		return (states >> BLOCK_VALUE_OFFSET) & BLOCK_VARIANT_MASK;
	}

	inline void setVariant(u_char8 variant) {
		states &= ~BLOCK_VARIANT_MASK << BLOCK_VARIANT_OFFSET;
		states |= variant << BLOCK_VARIANT_OFFSET;
	}


	inline u_char8 getCustomBits(u_char8 offset, u_char8 len) const {
		return (states >> (BLOCK_BITS_OFFSET + offset)) & ((1 << len) - 1);
	}

	inline void setCustomBits(u_char8 offset, u_char8 len, u_char8 value) {
		int len_musk = ((1 << len) - 1);
		offset += BLOCK_BITS_OFFSET;
		value &= len_musk;
		states &= ~(len_musk << offset);
		states |= value << offset;
	}


	// id of the state in range 0 to 3
	inline bool getCustomState(u_char8 id) const {
		return states & (1 << id+24);
	}

	// id of the state in range 0 to 3
	inline void setCustomState(u_char8 id, bool state) {
		if (state) {
			states |= (1 << id+24);
		} else {
			states &= ~(1 << id+24);
		}
	}

	// can use value in range 0 to 15
	inline u_char8 getCustomValue() const {
		return states >> 20;
	}

	/* if the block has a custom state you
	   can use value in range 0 to 15 else
	   you can use value in range 0 to 255 */
	inline void setCustomValue(u_char8 value) {
		states &= ~(0xF << 20);
		states |= (value << 20);
	}

	inline bool getSig() const {
		return (states >> BLOCK_SIG_OFFSET) & BLOCK_SIG_MASK;
	}

	inline void setSig(bool sig) {
		states &= ~(BLOCK_SIG_MASK << BLOCK_SIG_OFFSET);
		states |= sig << BLOCK_SIG_OFFSET;
	}
};

#endif /* VOXELS_VOXEL_H_ */
