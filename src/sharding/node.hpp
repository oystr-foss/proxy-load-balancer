//
// Created by realngnx on 11/06/2021.
//
#ifndef NODE_H
#define NODE_H

#include <iostream>


class Node {
public:
    virtual std::string get_key() = 0;

    std::string to_string() {
        return get_key();
    }

};

class ServiceNode: public Node {
public:
    ServiceNode() = default;
    ServiceNode(std::string service, std::string address, int port);

    std::string get_key() override;
    std::string get_host();
    int get_port();

private:
    std::string descriptor;
    std::string address;
    int port;

};

class VirtualNode: public Node {
public:
    VirtualNode() = default;
    VirtualNode(ServiceNode node, long replica_index);

    bool is_virtual_node_of(ServiceNode& node);
    std::shared_ptr<ServiceNode> get_physical_node();
    std::string get_key() override;

private:
    std::shared_ptr<ServiceNode> physical_node;
    long replica_index;

};


#endif //NODE_H
