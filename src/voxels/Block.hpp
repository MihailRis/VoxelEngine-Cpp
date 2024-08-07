#ifndef VOXELS_BLOCK_HPP_
#define VOXELS_BLOCK_HPP_

#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

#include <maths/UVRegion.hpp>
#include <maths/aabb.hpp>
#include <typedefs.hpp>

inline std::string BLOCK_ITEM_SUFFIX = ".item";

inline constexpr uint FACE_MX = 0;
inline constexpr uint FACE_PX = 1;
inline constexpr uint FACE_MY = 2;
inline constexpr uint FACE_PY = 3;
inline constexpr uint FACE_MZ = 4;
inline constexpr uint FACE_PZ = 5;

/// @brief Grid size used for physics solver collision checking with
/// complex hitboxes
inline constexpr uint BLOCK_AABB_GRID = 16;

inline std::string DEFAULT_MATERIAL = "base:stone";

struct block_funcs_set {
    bool init : 1;
    bool update : 1;
    bool onplaced : 1;
    bool onbroken : 1;
    bool oninteract : 1;
    bool randupdate : 1;
    bool onblockstick : 1;
};

struct CoordSystem {
    glm::ivec3 axisX;
    glm::ivec3 axisY;
    glm::ivec3 axisZ;

    /// @brief Grid 3d position fix offset (for negative vectors)
    glm::ivec3 fix;

    CoordSystem() = default;
    CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ);

    void transform(AABB& aabb) const;

    inline bool isVectorHasNegatives(glm::ivec3 vec) {
        return (vec.x < 0 || vec.y < 0 || vec.z < 0);
    }
};

struct BlockRotProfile {
    static const int MAX_COUNT = 8;
    std::string name;
    CoordSystem variants[MAX_COUNT];

    /// @brief No rotation
    static const BlockRotProfile NONE;

    /// @brief Wood logs, pillars, pipes
    static const BlockRotProfile PIPE;

    /// @brief Doors, signs and other panes
    static const BlockRotProfile PANE;

    static inline std::string PIPE_NAME = "pipe";
    static inline std::string PANE_NAME = "pane";
};

enum class BlockModel {
    /// @brief invisible
    none,
    /// @brief default cube shape
    block,
    /// @brief X-shape (grass)
    xsprite,
    /// @brief box shape sized as block hitbox
    aabb,
    /// @brief custom model defined in json
    custom
};

std::string to_string(BlockModel model);
std::optional<BlockModel> BlockModel_from(std::string_view str);

using BoxModel = AABB;

/// @brief Common kit of block properties applied to groups of blocks
struct BlockMaterial {
    std::string name;
    std::string stepsSound {""};
    std::string placeSound {""};
    std::string breakSound {""};
};

/// @brief Block properties definition
class Block {
public:
    /// @brief Block string id (with prefix included)
    std::string const name;

    std::string caption;

    /// @brief Textures set applied to block sides
    std::string textureFaces[6];  // -x,x, -y,y, -z,z

    std::vector<std::string> modelTextures = {};
    std::vector<BoxModel> modelBoxes = {};
    std::vector<glm::vec3> modelExtraPoints =
        {};                               // initially made for tetragons
    std::vector<UVRegion> modelUVs = {};  // boxes' tex-UVs also there

    /// @brief id of used BlockMaterial, may specify non-existing material
    std::string material = DEFAULT_MATERIAL;

    /// @brief Light emission R, G, B, S (sky lights: sun, moon, radioactive
    /// clouds)
    uint8_t emission[4] {0, 0, 0, 0};

    glm::i8vec3 size {1, 1, 1};

    /// @brief Influences visible block sides for transparent blocks
    uint8_t drawGroup = 0;

    /// @brief Block model type
    BlockModel model = BlockModel::block;

    /// @brief Does the block passing lights into itself
    bool lightPassing = false;

    /// @brief Does the block passing top-down sky lights into itself
    bool skyLightPassing = false;

    /// @brief Does block model have shading
    bool shadeless = false;

    /// @brief Does block model have vertex-based AO effect
    bool ambientOcclusion = true;

    /// @brief Is the block a physical obstacle
    bool obstacle = true;

    /// @brief Can the block be selected
    bool selectable = true;

    /// @brief Can the block be replaced with other.
    /// Examples of replaceable blocks: air, flower, water
    bool replaceable = false;

    /// @brief Can player destroy the block
    bool breakable = true;

    /// @brief Can the block be oriented different ways
    bool rotatable = false;

    /// @brief Can the block exist without physical support be a solid block
    /// below
    bool grounded = false;

    /// @brief Turns off block item generation
    bool hidden = false;

    /// @brief Set of block physical hitboxes
    std::vector<AABB> hitboxes;

    /// @brief Set of available block rotations (coord-systems)
    BlockRotProfile rotations = BlockRotProfile::NONE;

    /// @brief Item will be picked on MMB click on the block
    std::string pickingItem = name + BLOCK_ITEM_SUFFIX;

    /// @brief Block script name in blocks/ without extension
    std::string scriptName = name.substr(name.find(':') + 1);

    /// @brief Default block layout will be used by hud.open_block(...)
    std::string uiLayout = name;

    /// @brief Block inventory size. 0 - no inventory
    uint inventorySize = 0;

    // @brief Block tick interval (1 - 20tps, 2 - 10tps)
    uint tickInterval = 1;

    /// @brief Runtime indices (content indexing results)
    struct {
        /// @brief block runtime integer id
        blockid_t id;

        /// @brief is the block completely opaque for render and raycast
        bool solid = true;

        /// @brief does the block emit any lights
        bool emissive = false;

        // @brief block size is greather than 1x1x1
        bool extended = false;

        /// @brief set of hitboxes sets with all coord-systems precalculated
        std::vector<AABB> hitboxes[BlockRotProfile::MAX_COUNT];

        /// @brief set of block callbacks flags
        block_funcs_set funcsset {};

        /// @brief picking item integer id
        itemid_t pickingItem = 0;
    } rt {};

    Block(const std::string& name);
    Block(std::string name, const std::string& texture);
    Block(const Block&) = delete;
};

inline glm::ivec3 get_ground_direction(const Block& def, int rotation) {
    return -def.rotations.variants[rotation].axisY;
}

#endif  // VOXELS_BLOCK_HPP_
