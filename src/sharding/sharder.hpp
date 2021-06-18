//
// Created by realngnx on 11/06/2021.
//
#ifndef SHARDER_H
#define SHARDER_H

#include <boost/optional.hpp>
#include <map>
#include <iostream>
#include "./hasher.hpp"
#include "./node.hpp"


class Sharder {
public:
    virtual boost::optional<std::shared_ptr<ServiceNode>> route(std::string key) = 0;

};

class ConsistentHash: public Sharder {
public:
    ConsistentHash();
    ~ConsistentHash();

    boost::optional<std::shared_ptr<ServiceNode>> route(std::string key) override;
    void add(ServiceNode& node, long virtual_node_count);
    void remove(ServiceNode& node);
    void clear();
    long get_existing_replicas(ServiceNode& node);

private:
    Digest digest;
    std::map<long, VirtualNode*> ring;

    std::vector<long> extract_keys() {
        std::vector<long> keys;
        for (auto const& element : ring) {
            keys.push_back(element.first);
        }
        return keys;
    }
};


#endif //SHARDER_H
