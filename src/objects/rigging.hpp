#ifndef OBJECTS_SKELETON_HPP_
#define OBJECTS_SKELETON_HPP_

#include "../typedefs.hpp"

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

namespace rigging {
    struct Rig;

    struct Pose {
        std::vector<glm::mat4> matrices;
    };

    class RigNode {
        size_t index;
        std::string name;
        std::vector<std::unique_ptr<RigNode>> subnodes;
    public:
        RigNode(size_t index, std::string name, std::vector<std::unique_ptr<RigNode>> subnodes);

        size_t getIndex() const {
            return index;
        }

        const auto& getSubnodes() const {
            return subnodes;
        }
    };
    
    class RigConfig {
        std::unique_ptr<RigNode> root;
        std::unordered_map<std::string, size_t> indices;
        std::vector<RigNode*> nodes;
    public:
        RigConfig(std::unique_ptr<RigNode> root);
    };

    struct Rig {
        RigConfig* config;
        Pose pose;
        std::vector<std::string> textures;
    };
};

#endif // OBJECTS_SKELETON_HPP_
