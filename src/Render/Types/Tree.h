#pragma once
#include "Render/Types/Node.h"
#include <vector>

struct Tree {
    private:
    std::vector<Node> m_nodes;
    size_t m_max_nodes = 0;

    public:
    uint32_t m_root_node_id = UINT32_MAX;
    uint32_t m_last_root_node_id = UINT32_MAX;
    
    void init(size_t max_nodes);
    std::pair<Node&, uint32_t> createNode();
    Node* getNode(uint32_t index);
};
