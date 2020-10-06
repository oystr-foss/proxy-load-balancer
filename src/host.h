#include <iostream>

#ifndef OLPB_HOST_H
#define OLPB_HOST_H

class Host {
public:
    std::string host;
    int port;

    Host(std::string host, int port) {
        this->host = std::move(host);
        this->port = port;
    }
};

#endif //OLPB_HOST_H
