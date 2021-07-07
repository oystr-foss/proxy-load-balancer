//
// Created by realngnx on 11/06/2021.
//
#include <memory>
#include "./sharder.hpp"


ConsistentHash::~ConsistentHash() {
    ring.clear();
}

ConsistentHash::ConsistentHash() {
    std::map<long, VirtualNode*> hosts_map {};
    ring = hosts_map;
}

boost::optional<std::shared_ptr<ServiceNode>> ConsistentHash::route(std::string key) {
    if(ring.empty()) {
        boost::optional<std::shared_ptr<ServiceNode>> opt;

        return opt;
    }

    long hash_val = digest.to_md5_hash(key);
    std::map<long, VirtualNode*> tail_map;
    for(auto n : ring) {
        if(n.first >= hash_val) {
            tail_map[n.first] = n.second;
        }
    }

    try {
        long node_hash_val = !tail_map.empty() ? tail_map.begin()->first : ring.begin()->first;
        auto route_to = ring[node_hash_val]->get_physical_node();

        return boost::make_optional(route_to);
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;

        return boost::make_optional(ring.begin()->second->get_physical_node());
    }
}

void ConsistentHash::add(ServiceNode& node, long virtual_node_count) {
    if(virtual_node_count < 0) {
        std::cout << "Virtual node count is < 0!" << std::endl;
        return;
    }

    long existing_replicas = get_existing_replicas(node);

    for(long i = 0; i < virtual_node_count; i++) {
        long index = i + existing_replicas;
        auto v_node = new VirtualNode(node, index);
        long hash = digest.to_md5_hash(v_node->get_key());
        ring[hash] = v_node;
    }
    std::cout << "Added! " << node.get_key() << std::endl;
}

void ConsistentHash::remove(ServiceNode& node) {
    std::map<long, VirtualNode*> ring2(ring);
    if(ring2.empty()) {
        return;
    }

    for(auto const & tup : ring2) {
        if((*tup.second).is_virtual_node_of(node)) {
            auto iterator = ring.find(tup.first);
            ring.erase(iterator);
        }
    }

    std::cout << "Removed " << node.get_key() << std::endl;
}

void ConsistentHash::clear() {
    ring.clear();
}

long ConsistentHash::get_existing_replicas(ServiceNode& node) {
    long count = 0;
    auto keys = extract_keys();

    for(long key : keys) {
        VirtualNode* n = ring[key];
        if(n->is_virtual_node_of(node)) {
            count += 1;
        }
    }

    return count;
}
