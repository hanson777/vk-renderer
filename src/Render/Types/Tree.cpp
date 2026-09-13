#include "Tree.h"
#include <cstdint>
#include <iostream>

void Tree::init(size_t max_nodes) {
    m_max_nodes = max_nodes;
    m_nodes.reserve(max_nodes);
}

std::pair<Node&, uint32_t> Tree::createNode() {
    uint32_t id = m_nodes.size();
    m_nodes.push_back(Node{});
    return { m_nodes[id], id };
};

Node* Tree::getNode(uint32_t id) {
    if (id < 0 || id >= m_nodes.size()) {
        std::cerr << "[ERROR::TREE] getNode out of bounds\n";
        return nullptr;
    }
    return &m_nodes[id];
}
