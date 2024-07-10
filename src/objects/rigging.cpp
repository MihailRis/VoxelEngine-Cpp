#include "rigging.hpp"

#include "../assets/Assets.hpp"
#include "../graphics/render/ModelBatch.hpp"
#include "../graphics/core/Model.hpp"
#include "../coders/json.hpp"

using namespace rigging;

Bone::Bone(
    size_t index, 
    std::string name, 
    std::string model,
    std::vector<std::unique_ptr<Bone>> bones)
  : index(index), 
    name(std::move(name)),
    modelName(std::move(model)),
    bones(std::move(bones)) 
{}

void Bone::setModel(const std::string& name) {
    if (modelName == name) {
        return;
    }
    modelName = name;
    modelUpdated = true;
}

void Bone::refreshModel(const Assets* assets) {
    if (modelUpdated) {
        model = assets->get<model::Model>(modelName);
        modelUpdated = false;
    }
}

static void get_all_nodes(std::vector<Bone*>& nodes, Bone* node) {
    nodes[node->getIndex()] = node;
    for (auto& subnode : node->getSubnodes()) {
        get_all_nodes(nodes, subnode.get());
    }
}

SkeletonConfig::SkeletonConfig(const std::string& name, std::unique_ptr<Bone> root, size_t nodesCount)
  : name(name), root(std::move(root)), nodes(nodesCount) {
    get_all_nodes(nodes, this->root.get());
}

size_t SkeletonConfig::update(
    size_t index, 
    Skeleton& skeleton, 
    Bone* node, 
    glm::mat4 matrix) const
{
    skeleton.calculated.matrices[index] = matrix * skeleton.pose.matrices[index];
    size_t count = 1;
    for (auto& subnode : node->getSubnodes()) {
        count += update(index+count, skeleton, subnode.get(), skeleton.calculated.matrices[index]);
    }
    return count;
}

void SkeletonConfig::update(Skeleton& skeleton, glm::mat4 matrix) const {
    update(0, skeleton, root.get(), matrix);
}

void SkeletonConfig::render(
    Assets* assets,
    ModelBatch& batch,
    Skeleton& skeleton,
    const glm::mat4& matrix) const
{
    update(skeleton, matrix);
    for (size_t i = 0; i < nodes.size(); i++) {
        auto* node = nodes[i];
        node->refreshModel(assets);
        if (auto model = node->getModel()) {
            batch.pushMatrix(skeleton.calculated.matrices[i]);
            batch.draw(model, &skeleton.textures);
            batch.popMatrix();
        }
    }
}

static std::tuple<size_t, std::unique_ptr<Bone>> read_node(
    dynamic::Map* root, size_t index
) {
    std::string name;
    std::string model;
    root->str("name", name);
    root->str("model", model);
    std::vector<std::unique_ptr<Bone>> bones;
    size_t count = 1;
    if (auto nodesList = root->list("nodes")) {
        for (size_t i = 0; i < nodesList->size(); i++) {
            if (const auto& map = nodesList->map(i)) {
                auto [subcount, subNode] = read_node(map.get(), index+count);
                subcount += count;
                bones.push_back(std::move(subNode));
            }
        }
    }
    return {index + count, std::make_unique<Bone>(index, name, model, std::move(bones))};
}

std::unique_ptr<SkeletonConfig> SkeletonConfig::parse(
    std::string_view src,
    std::string_view file,
    std::string_view name
) {
    auto root = json::parse(file, src);
    auto rootNodeMap = root->map("root");
    if (rootNodeMap == nullptr) {
        throw std::runtime_error("missing 'root' element");
    }
    auto [count, rootNode] = read_node(rootNodeMap.get(), 0);
    return std::make_unique<SkeletonConfig>(std::string(name), std::move(rootNode), count);
}
