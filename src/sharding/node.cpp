//
// Created by realngnx on 11/06/2021.
//
#include <boost/algorithm/string.hpp>
#include <utility>
#include <boost/format.hpp>
#include "./node.hpp"


ServiceNode::ServiceNode(std::string service, std::string address, int port):
    descriptor(std::move(service)),
    address(std::move(address)),
    port(port) {}

std::string ServiceNode::get_key() {
    auto format = boost::format("%1%:%2%:%3%") % this->descriptor % this->address % this->port;
    return boost::str(format);
}

std::string ServiceNode::get_host() {
    return this->address;
}

int ServiceNode::get_port() {
    return this->port;
}

VirtualNode::VirtualNode(ServiceNode node, long replica_index):
    physical_node(std::make_shared<ServiceNode>(node)),
    replica_index(replica_index) {}

std::string VirtualNode::get_key() {
    auto format = boost::format("%1%:%2%") % get_physical_node()->get_key() % replica_index;
    return boost::str(format);
}

bool VirtualNode::is_virtual_node_of(ServiceNode& node) {
    std::string key = node.get_key();
    return boost::iequals(key, get_physical_node()->get_key());
}

std::shared_ptr<ServiceNode> VirtualNode::get_physical_node() {
    return physical_node;
}
