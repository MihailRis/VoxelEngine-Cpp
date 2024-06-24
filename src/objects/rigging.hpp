#ifndef OBJECTS_SKELETON_HPP_
#define OBJECTS_SKELETON_HPP_

#include "../typedefs.hpp"

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace rigging {
    struct Rig;

    struct Pose {
        std::vector<glm::mat4> matrices;
    };

    class RigNode {
        uint index;
        std::vector<std::unique_ptr<RigNode>> subnodes;
    public:
        RigNode(uint index);
    };
    
    class RigConfig {
        std::unique_ptr<RigNode> root;
    };

    struct Rig {
        RigConfig* config;
        Pose pose;
    };
};

#endif // OBJECTS_SKELETON_HPP_
