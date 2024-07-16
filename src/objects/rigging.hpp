#ifndef OBJECTS_RIGGING_HPP_
#define OBJECTS_RIGGING_HPP_

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
    struct Skeleton;
    class SkeletonConfig;

    struct Pose {
        std::vector<glm::mat4> matrices;

        Pose(size_t size) {
            matrices.resize(size, glm::mat4(1.0f));
        }
    };

    struct ModelReference {
        std::string name;
        model::Model* model;
        bool updateFlag;

        void refresh(const Assets* assets);
    };

    class Bone {
        size_t index;
        std::string name;
        std::vector<std::unique_ptr<Bone>> bones;
    public:
        ModelReference model;
        Bone(
            size_t index, 
            std::string name,
            std::string model,
            std::vector<std::unique_ptr<Bone>> bones);

        void setModel(const std::string& name);

        const std::string& getName() const {
            return name;
        }
        
        size_t getIndex() const {
            return index;
        }

        const auto& getSubnodes() const {
            return bones;
        }
    };

    struct BoneFlags {
        bool visible: 1;
    };

    struct Skeleton {
        const SkeletonConfig* config;
        Pose pose;
        Pose calculated;
        std::vector<BoneFlags> flags;
        std::unordered_map<std::string, std::string> textures;
        std::vector<ModelReference> modelOverrides;
        bool visible;

        Skeleton(const SkeletonConfig* config);
    };
    
    class SkeletonConfig {
        std::string name;
        std::unique_ptr<Bone> root;
        std::unordered_map<std::string, size_t> indices;
        
        /// Nodes and indices are ordered from root to bones.
        /// Example:
        /// 0 - root
        /// 1 --- sub1
        /// 2 ----- subsub1
        /// 3 --- sub2
        std::vector<Bone*> nodes;

        size_t update(
            size_t index,
            Skeleton& skeleton,
            Bone* node,
            glm::mat4 matrix) const;
    public:
        SkeletonConfig(const std::string& name, std::unique_ptr<Bone> root, 
                  size_t nodesCount);

        void update(Skeleton& skeleton, glm::mat4 matrix) const;
        void render(
            Assets* assets,
            ModelBatch& batch,
            Skeleton& skeleton, 
            const glm::mat4& matrix) const;

        Skeleton instance() const {
            return Skeleton(this);
        }

        Bone* find(std::string_view str) const;

        static std::unique_ptr<SkeletonConfig> parse(
            std::string_view src,
            std::string_view file,
            std::string_view name
        );

        const std::vector<Bone*>& getNodes() const {
            return nodes;
        }
        
        const std::string& getName() const {
            return name;
        }
    };
};

#endif // OBJECTS_RIGGING_HPP_
