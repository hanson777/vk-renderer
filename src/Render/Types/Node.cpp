#include "Node.h"
#include <cstdint>
#include <iostream>

uint32_t Tree::addNode(Node& node) {
    nodes.push_back(node);
    return nodes.size() - 1;
};

Node* Tree::getNodeByIndex(uint32_t index) {
    if (index < 0 || index >= nodes.size()) {
        std::cerr << "[ERROR::NODETREE] getNodeByIndex out of bounds\n";
        return nullptr;
    }
    return &nodes[index];
}
