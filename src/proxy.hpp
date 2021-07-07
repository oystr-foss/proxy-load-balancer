//
// Created by realngnx on 11/06/2021.
//

#ifndef OPLB_PROXY_HPP
#define OPLB_PROXY_HPP

#include <memory>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <list>
#include <random>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/io_service.hpp>

#include <curl/curl.h>

#include <jsoncpp/json/json.h>
#include <wait.h>
#include <ctime>

#include "timer.hpp"
#include "util.hpp"
#include "sharding/node.hpp"
#include "sharding/sharder.hpp"


void get_available_nodes(const boost::system::error_code & /*e*/, timer *scheduler,
    std::map<std::string, std::string> &config, ConsistentHash& sharder);

namespace {
    std::size_t callback(
        const char *in,
        std::size_t size,
        std::size_t num,
        std::string *out
    );
}

namespace tcp_proxy {
    namespace ip = boost::asio::ip;

    class bridge : public boost::enable_shared_from_this<bridge> {
    public:
        typedef ip::tcp::socket socket_type;
        typedef boost::shared_ptr<bridge> ptr_type;

        explicit bridge(boost::asio::io_service &ios);

        socket_type downstream_socket_;
        socket_type upstream_socket_;

        socket_type &downstream_socket();
        socket_type &upstream_socket();
        void start(const std::string &upstream_host, unsigned short upstream_port);
        void handle_upstream_connect(const boost::system::error_code &error);

    private:

        // Read from remote server complete, send data to client
        void handle_upstream_read(const boost::system::error_code &error, const size_t &bytes_transferred);
        void handle_downstream_write(const boost::system::error_code &error);
        void handle_downstream_read(const boost::system::error_code &error, const size_t &bytes_transferred);
        void handle_upstream_write(const boost::system::error_code &error);
        void close();

        enum {
            max_data_length = 8196 // 8KB
        };
        unsigned char downstream_data_[max_data_length];
        unsigned char upstream_data_[max_data_length];

        boost::mutex mutex_;

    public:
        class acceptor {
        public:
            acceptor(boost::asio::io_service &io_service,
                     const std::string &local_host,
                     unsigned short local_port,
                     timer &t,
                     std::map<std::string, std::string> &config,
                     ConsistentHash& sharder);

            bool accept_connections();

        private:
            void handle_accept(const boost::system::error_code &error);
            void service_unavailable();

            boost::asio::io_service &io_service_;
            ip::address_v4 localhost_address;
            ip::tcp::acceptor acceptor_;
            ptr_type session_;
            ConsistentHash& sharder_;

        };
    };
}

#endif //OPLB_PROXY_HPP
