#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#include <boost/asio/io_service.hpp>

#include "util.hpp"
#include "timer.hpp"
#include "proxy.hpp"

// TODO:
// * Limit forking and reuse childs instead of killing them.
int main(int argc, char *argv[]) {
    std::map<std::string, std::string> config;

    if(argc > 1) {
        read_config(config, argv[1]);
    } else {
        read_config(config);
    }

    if(config.count("log_file") >= 1) {
        // Logging to specified file.
        freopen(config["log_file"].c_str(), "w", stdout);
        freopen(config["log_file"].c_str(), "w", stderr);
    }

    if (config.count("port") != 1) {
        std::cerr << "a port must be specified in proxy.conf" << std::endl;
        exit(1);
    }

    std::string local_host;
    unsigned short local_port;

    try {
        local_host = "0.0.0.0";
        if (config.count("host") == 1) {
            local_host = config["host"];
        }

        local_port = static_cast<unsigned short>(boost::lexical_cast<int>(config["port"]));
    } catch (boost::bad_lexical_cast const &e) {
        std::cerr << "Invalid port" << std::endl;
        exit(1);
    }

    boost::asio::io_service ios;
    timer t(ios, config);

    try {
        auto sharder = ConsistentHash();
        tcp_proxy::bridge::acceptor acceptor(ios, local_host, local_port, t, config, sharder);
        std::cout << "Listening on: " << local_host << ":" << local_port << "\n" << std::endl;

        ios.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
