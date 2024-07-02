#ifndef OBJECTS_SKELETON_HPP_
#define OBJECTS_SKELETON_HPP_

#include "../typedefs.hpp"

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

class Assets;
class ModelBatch;

namespace model {
    struct Model;
}

namespace rigging {
    struct Rig;
    class RigConfig;

    struct Pose {
        std::vector<glm::mat4> matrices;

        Pose(size_t size) : matrices(size) {
            matrices.resize(size);
            for (size_t i = 0; i < size; i++) {
                matrices[i] = glm::mat4(1.0f);
            }
        }
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

        model::Model* getModel() const {
            return model;
        }

        size_t getIndex() const {
            return index;
        }

        const auto& getSubnodes() const {
            return subnodes;
        }
    };

    struct Rig {
        RigConfig* config;
        Pose pose;
        Pose calculated;
        std::vector<std::string> textures;
    };
    
    class RigConfig {
        std::unique_ptr<RigNode> root;
        std::unordered_map<std::string, size_t> indices;
        std::vector<RigNode*> nodes;

        size_t update(
            size_t index,
            Rig& rig,
            RigNode* node,
            glm::mat4 matrix) const;
    public:
        RigConfig(std::unique_ptr<RigNode> root, size_t nodesCount);

        void update(Rig& rig, glm::mat4 matrix) const;
        void setup(const Assets* assets, RigNode* node=nullptr) const;
        void render(
            Assets* assets,
            ModelBatch& batch,
            Rig& rig, 
            const glm::mat4& matrix) const;

        Rig instance() {
            return Rig {
                this, Pose(nodes.size()), Pose(nodes.size()), {}
            };
        }

        static std::unique_ptr<RigConfig> parse(
            std::string_view src,
            std::string_view file
        );

        const std::vector<RigNode*>& getNodes() const {
            return nodes;
        }
    };
};

#endif // OBJECTS_SKELETON_HPP_
