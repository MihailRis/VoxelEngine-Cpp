#ifndef OBJECTS_SKELETON_HPP_
#define OBJECTS_SKELETON_HPP_

#include "../typedefs.hpp"

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

class Assets;

namespace model {
    struct Model;
}

namespace rigging {
    struct Rig;

    struct Pose {
        std::vector<glm::mat4> matrices;
    };

    class RigNode {
        size_t index;
        std::string name;
        std::string modelName;
        std::vector<std::unique_ptr<RigNode>> subnodes;
        model::Model* model = nullptr;
    public:
        RigNode(
            size_t index, 
            std::string name,
            std::string model,
            std::vector<std::unique_ptr<RigNode>> subnodes);

        void setModel(const Assets* assets, const std::string& name);

        const std::string& getModelName() const {
            return modelName;
        }

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

        size_t update(
            size_t index,
            Rig& rig,
            RigNode* node,
            glm::mat4 matrix);
    public:
        RigConfig(std::unique_ptr<RigNode> root);

        void update(Rig& rig, glm::mat4 matrix);
        void setup(const Assets* assets, RigNode* node=nullptr);

        static std::unique_ptr<RigConfig> parse(
            std::string_view src,
            std::string_view file
        );
    };

    struct Rig {
        RigConfig* config;
        Pose pose;
        Pose calculated;
        std::vector<std::string> textures;
    };
};

#endif // OBJECTS_SKELETON_HPP_
