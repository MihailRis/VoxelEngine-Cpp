#include "rigging.hpp"

#include "../assets/Assets.hpp"
#include "../graphics/render/ModelBatch.hpp"
#include "../graphics/core/Model.hpp"
#include "../coders/json.hpp"

using namespace rigging;

RigNode::RigNode(
    size_t index, 
    std::string name, 
    std::string model,
    std::vector<std::unique_ptr<RigNode>> subnodes)
  : index(index), 
    name(std::move(name)),
    modelName(std::move(model)),
    subnodes(std::move(subnodes)) 
{}

void RigNode::setModel(const Assets* assets, const std::string& name) {
    modelName = name;
    model = assets->get<model::Model>(name);
}

static void get_all_nodes(std::vector<RigNode*>& nodes, RigNode* node) {
    nodes[node->getIndex()] = node;
    for (auto& subnode : node->getSubnodes()) {
        get_all_nodes(nodes, subnode.get());
    }
}

RigConfig::RigConfig(const std::string& name, std::unique_ptr<RigNode> root, size_t nodesCount)
  : name(name), root(std::move(root)), nodes(nodesCount) {
    get_all_nodes(nodes, this->root.get());
}

size_t RigConfig::update(
    size_t index, 
    Rig& rig, 
    RigNode* node, 
    glm::mat4 matrix) const
{
    rig.calculated.matrices[index] = matrix * rig.pose.matrices[index];
    size_t count = 1;
    for (auto& subnode : node->getSubnodes()) {
        count += update(index+count, rig, subnode.get(), rig.calculated.matrices[index]);
    }
    return count;
}

void RigConfig::update(Rig& rig, glm::mat4 matrix) const {
    update(0, rig, root.get(), matrix);
}

void RigConfig::setup(const Assets* assets, RigNode* node) const {
    if (node == nullptr) {
        setup(assets, root.get());
    } else {
        node->setModel(assets, node->getModelName());
        for (auto& subnode : node->getSubnodes()) {
            setup(assets, subnode.get());
        }
    }
}

void RigConfig::render(
    Assets*,
    ModelBatch& batch,
    Rig& rig,
    const glm::mat4& matrix) const
{
    update(rig, matrix);
    for (size_t i = 0; i < nodes.size(); i++) {
        auto* node = nodes[i];
        auto model = node->getModel();
        if (model == nullptr) {
            continue;
        }
        batch.pushMatrix(rig.calculated.matrices[i]);
        batch.draw(model, &rig.textures);
        batch.popMatrix();
    }
}

static std::tuple<size_t, std::unique_ptr<RigNode>> read_node(
    dynamic::Map* root, size_t index
) {
    std::string name;
    std::string model;
    root->str("name", name);
    root->str("model", model);
    std::vector<std::unique_ptr<RigNode>> subnodes;
    size_t count = 1;
    if (auto nodesList = root->list("nodes")) {
        for (size_t i = 0; i < nodesList->size(); i++) {
            if (auto map = nodesList->map(i)) {
                auto [subcount, subNode] = read_node(map, index+count);
                subcount += count;
                subnodes.push_back(std::move(subNode));
            }
        }
    }
    return {index + count, std::make_unique<RigNode>(index, name, model, std::move(subnodes))};
}

std::unique_ptr<RigConfig> RigConfig::parse(
    std::string_view src,
    std::string_view file,
    std::string_view name
) {
    auto root = json::parse(file, src);
    auto rootNodeMap = root->map("root");
    if (rootNodeMap == nullptr) {
        throw std::runtime_error("missing 'root' element");
    }
    auto [count, rootNode] = read_node(rootNodeMap, 0);
    return std::make_unique<RigConfig>(std::string(name), std::move(rootNode), count);
}
