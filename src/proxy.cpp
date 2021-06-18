#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-avoid-bind"
//
// Created by realngnx on 11/06/2021.
//

#include "proxy.hpp"
#include <utility>


namespace {
    std::size_t callback(
            const char *in,
            std::size_t size,
            std::size_t num,
            std::string *out) {
        const std::size_t total_bytes(size * num);
        out->append(in, total_bytes);
        return total_bytes;
    }
}

namespace tcp_proxy {
    bridge::bridge(boost::asio::io_service &ios):
        downstream_socket_(ios),
        upstream_socket_(ios) {}

    bridge::socket_type &bridge::downstream_socket() {
        // Client socket
        return downstream_socket_;
    }

    bridge::socket_type &bridge::upstream_socket() {
        // Remote socket
        return upstream_socket_;
    }

    void bridge::start(const std::string &upstream_host, unsigned short upstream_port) {
        upstream_socket_.async_connect(
            ip::tcp::endpoint(ip::address::from_string(upstream_host), upstream_port),
            boost::bind(&bridge::handle_upstream_connect,shared_from_this(), boost::asio::placeholders::error));
    }

    void bridge::handle_upstream_connect(const boost::system::error_code &error) {
        if (!error) {
            // Setup async read from upstream
            upstream_socket_.async_read_some(
                    boost::asio::buffer(upstream_data_, max_data_length),
                    boost::bind(&bridge::handle_upstream_read,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));

            downstream_socket_.async_read_some(
                    boost::asio::buffer(downstream_data_, max_data_length),
                    boost::bind(&bridge::handle_downstream_read,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        } else {
            std::cout << "upstream_connect: " << error.message() << std::endl;
            close();
        }
    }

    void bridge::handle_upstream_read(const boost::system::error_code &error, const size_t &bytes_transferred) {
        if (!error) {
            async_write(downstream_socket_, boost::asio::buffer(upstream_data_, bytes_transferred),
                        boost::bind(&bridge::handle_downstream_write,
                                    shared_from_this(),
                                    boost::asio::placeholders::error));
        } else {
            if (!closed) {
                closed = true;
                close();
                exit(0);
            }
        }
    }

    void bridge::handle_downstream_write(const boost::system::error_code &error) {
        if (!error) {
            upstream_socket_.async_read_some(
                    boost::asio::buffer(upstream_data_, max_data_length),
                    boost::bind(&bridge::handle_upstream_read, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        } else {
            if (!closed) {
                closed = true;
                close();
                exit(0);
            }
        }
    }

    void bridge::handle_downstream_read(const boost::system::error_code &error, const size_t &bytes_transferred) {
        if (!error) {
            async_write(upstream_socket_, boost::asio::buffer(downstream_data_, bytes_transferred),
                        boost::bind(&bridge::handle_upstream_write, shared_from_this(),
                                    boost::asio::placeholders::error));
        } else {
            if (!closed) {
                closed = true;
                close();
                exit(0);
            }
        }
    }

    void bridge::handle_upstream_write(const boost::system::error_code &error) {
        if (!error) {
            downstream_socket_.async_read_some(
                    boost::asio::buffer(downstream_data_, max_data_length),
                    boost::bind(&bridge::handle_downstream_read, shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
        } else {
            if (!closed) {
                closed = true;
                close();
                exit(0);
            }
        }
    }

    void bridge::close() {
        boost::mutex::scoped_lock lock(mutex_);

        if (downstream_socket_.is_open()) {
            downstream_socket_.close();
        }

        if (upstream_socket_.is_open()) {
            upstream_socket_.close();
        }
        exit(0);
    }

    bridge::acceptor::acceptor(
        boost::asio::io_service &io_service,
        const std::string &local_host,
        unsigned short local_port,
        timer &t,
        std::map<std::string, std::string> &config,
        ConsistentHash& sharder
    ):
        io_service_(io_service),
        localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
        acceptor_(io_service_, ip::tcp::endpoint(localhost_address, local_port)),
        ppid(-1),
        signal_(io_service, SIGCHLD),
        sharder_(sharder) {

        const boost::system::error_code null;
        get_available_nodes(null, &t, config, sharder_);
        wait_for_signal();
        accept_connections();
    }

    void bridge::acceptor::wait_for_signal() {
        signal_.async_wait(boost::bind(&acceptor::handle_signal_wait, this));
    }

    void bridge::acceptor::handle_signal_wait() {
        if (getpid() == ppid) {
            int status = 0;
            waitpid(-1, &status, WNOHANG > 0);
            wait_for_signal();
        }
    }

    bool bridge::acceptor::accept_connections() {
        try {
            session_ = boost::shared_ptr<bridge>(new bridge(io_service_));
            acceptor_.async_accept(session_->downstream_socket(),
                                   boost::bind(&acceptor::handle_accept,
                                               this,
                                               boost::asio::placeholders::error));
            return true;
        } catch (std::exception &e) {
            std::cerr << "acceptor exception: " << e.what() << std::endl;
            return false;
        }
    }

    void bridge::acceptor::handle_accept(const boost::system::error_code &error) {
        if (!error) {
            io_service_.notify_fork(boost::asio::io_service::fork_prepare);
            auto pid = fork();

            if (pid == 0) {
                auto remote = session_->downstream_socket_.remote_endpoint().address().to_string();
                auto n = sharder_.route(remote);

                if(!n.has_value()) {
                    service_unavailable();
                    return;
                }

                const std::string host(n.value()->get_host());
                const unsigned short port(n.value()->get_port());

                io_service_.notify_fork(boost::asio::io_service::fork_child);
                std::cout << "[" << get_date("%d/%m/%Y %H:%M:%S") << "] " <<
                    session_->downstream_socket_.remote_endpoint() << " -> " << host << ":" << port << std::endl;

                acceptor_.close();
                signal_.cancel();
                session_->start(host, port);
            } else {
                auto remote = session_->downstream_socket_.remote_endpoint().address().to_string();
                io_service_.notify_fork(boost::asio::io_service::fork_parent);
                ppid = getpid();
                int status = 0;
                waitpid(pid, &status, WNOHANG > 0);
                accept_connections();
            }
        } else {
            std::cerr << "Error: " << error.message() << std::endl;
            accept_connections();
        }
    }

    void bridge::acceptor::service_unavailable() {
        std::string message("No hosts available to forward requests to.");
        std::cout << message << std::endl;
        std::string contentLength = std::to_string(message.length());
        std::string response = "HTTP/1.1 503 Service Unavailable\nX-Oystr-Proxy: true\nContent-Type:"
                               " text/html; charset=utf-8\nContent-Length: " + contentLength + "\n\n" + message;
        this->session_->downstream_socket().write_some(boost::asio::buffer(response));
        this->accept_connections();
    }
}

// TODO: Implement a HTTP Client.
void get_available_nodes(const boost::system::error_code & /*e*/, timer *scheduler,
                         std::map<std::string, std::string> &config, ConsistentHash& sharder) {

    const std::string url(config["discovery_url"]);
    const std::string endpoint(config["endpoint"]);
    const std::string final_url(url + endpoint);

    CURL *curl;
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, final_url.c_str());
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        long http_code(0);
        std::unique_ptr<std::string> http_data(new std::string());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, http_data.get());

        curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);

        if (http_code != 200) {
            std::cerr << "Non-200 response (" << http_code << "): " << http_data->c_str() << std::endl;
        } else {
            Json::Value data;
            Json::Reader reader;

            if (reader.parse(*http_data, data)) {
                // Cleaning up to get rid of unhealthy nodes/peers.
                sharder.clear();
                for (auto item : data) {
                    const std::string service_id(item["service_id"].asString());
                    const std::string host(item["host"].asString());
                    const unsigned short port(item["port"].asInt());

                    ServiceNode node(service_id, host, port);

                    if(sharder.get_existing_replicas(node) == 0) {
                        sharder.add(node, 10);
                    }
                }
            } else {
                std::cout << "Non JSON response: " << http_data->c_str() << std::endl;
            }
        }
    }

    scheduler->schedule(get_available_nodes, sharder);
}


#pragma clang diagnostic pop
