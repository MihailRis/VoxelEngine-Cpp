#include "rigging.hpp"

using namespace rigging;

RigNode::RigNode(size_t index, std::string name, std::vector<std::unique_ptr<RigNode>> subnodes)
  : index(index), name(std::move(name)), subnodes(std::move(subnodes)) {
}

RigConfig::RigConfig(std::unique_ptr<RigNode> root) : root(std::move(root)) {
}
