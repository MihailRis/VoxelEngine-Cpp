#include "rigging.hpp"

#include "../assets/Assets.hpp"
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
    modelName(model),
    subnodes(std::move(subnodes)) 
{}

void RigNode::setModel(const Assets* assets, const std::string& name) {
    modelName = name;
    model = assets->get<model::Model>(name);
}

RigConfig::RigConfig(std::unique_ptr<RigNode> root) : root(std::move(root)) {
}

size_t RigConfig::update(
    size_t index, 
    Rig& rig, 
    RigNode* node, 
    glm::mat4 matrix) 
{
    rig.calculated.matrices[index] = matrix * rig.pose.matrices[index];
    index++;
    for (auto& subnode : node->getSubnodes()) {
        index = update(index, rig, subnode.get(), rig.calculated.matrices[index]);
    }
    return index;
}

void RigConfig::update(Rig& rig, glm::mat4 matrix) {
    update(0, rig, root.get(), matrix);
}

void RigConfig::setup(const Assets* assets, RigNode* node) {
    if (node == nullptr) {
        setup(assets, root.get());
    } else {
        node->setModel(assets, node->getModelName());
        for (auto& subnode : node->getSubnodes()) {
            setup(assets, subnode.get());
        }
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
    return {index, std::make_unique<RigNode>(index, name, model, std::move(subnodes))};
}

std::unique_ptr<RigConfig> RigConfig::parse(
    std::string_view src,
    std::string_view file
) {
    auto root = json::parse(file, src);
    auto rootNodeMap = root->map("root");
    if (rootNodeMap == nullptr) {
        throw std::runtime_error("missing 'root' element");
    }
    auto [count, rootNode] = read_node(root.get(), 0);
    return std::make_unique<RigConfig>(std::move(rootNode));
}
