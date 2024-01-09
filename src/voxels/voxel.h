#ifndef VOXELS_VOXEL_H_
#define VOXELS_VOXEL_H_

#include "../typedefs.h"
#include <bit>
#include <cstring>

namespace std {
	#ifndef __cpp_lib_bit_cast
	
	// #include <type_traits>

	template<class To, class From>
	enable_if_t<
		sizeof(To) == sizeof(From) &&
		is_trivially_copyable_v<From> &&
		is_trivially_copyable_v<To>,
		To>
	// constexpr support needs compiler magic
	bit_cast(const From& src) noexcept
	{
		// static_assert(is_trivially_constructible_v<To>,
		// 	"This implementation additionally requires "
		// 	"destination type to be trivially constructible");
	
		To dst;
		memcpy(&dst, &src, sizeof(To));
		return dst;
	}
	#endif
}


/**
 *    ,------------------------------------------------------------------------------------------------------------------------------------,
 *    |                                                                                                                                    |
 *    |                     ,---------------------,                                                                                        |
 *    |                     |  OLD VOXEL BIT MAP  |                                                                                        |
 *    |                     '---------------------'                                                                                        |
 *    | 0                               16    19 20     24      28      32                                                                 |
 *    | |                               |     | /       |       |       |                                                                  |
 *    | |# # # # # # # # # # # # # # # #|# # #|#|# # # #|# # # #|#'#'#'#|                                                                  |
 *    |                 id                dir / \ variant  value  4states                                                                  |
 *    |                                      |sig|                                                                                         |
 *    |                                                                                                                                    |
 *    |                                                                                                                                    |
 *    |                                                      ,---------------------,                                                       |
 *:   |                                                      |  NEW VOXEL BIT MAP  |                                                       |
 *    |                                                      '---------------------'                                                       |
 *    | 0                               16              24      28      32              40              48              56              64 |
 *    | |                               |               |       |       |               :               :               :               |  |
 *    | |# # # # # # # # # # # # # # # #'# # # # # # # #|# # # #|# # # #|# # # # # # # #'# # # # # # # #'# # # # # # # #'# # # # # # # #|  |
 *    |                 id                   variant      dir     signal                        custom bits states                         |
 *    |                                                                                                                                    |
 *    '------------------------------------------------------------------------------------------------------------------------------------'
 */

enum class VOX_DIR:voxel_t {
	NORTH,
	WEST,
	SOUTH,
	EAST,
	UP,
	DOWN
};

enum class VOX_ROT:voxel_t {
	NN,
	WW,
	SS,
	EE,
	NW,
	WS,
	SE,
	EN
};

struct voxel
{
	voxel_t id:16 = 0;
	voxel_t variant:8 = 0;
	voxel_t dir:4 = 0;
	voxel_t signal:4 = 0;
	voxel_t bits:32 = 0;

	voxel() = default;

	voxel(voxel_t vox) : voxel(std::bit_cast<voxel>(vox)) {

	}

	explicit operator voxel_t() const {
		return std::bit_cast<voxel_t>(*this);
	};

	inline u_char getCustomBits(u_char offset, u_char len) const {
		return (bits >> (offset)) & ((1 << len) - 1);
	}

	inline void setCustomBits(u_char offset, u_char len, u_char value) {
		int mask = ((1 << len) - 1);
		value &= mask;
		bits &= ~(mask << offset);
		bits |= value << offset;
	}

	bool operator==(const voxel& other) const {
		return id == other.id && variant == other.variant && dir == other.dir && signal == other.signal && bits == other.bits;
	}

	bool operator!=(const voxel& other) const {
        return!(*this == other);
    }
};

#endif /* VOXELS_VOXEL_H_ */
