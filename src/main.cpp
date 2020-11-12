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

#include <json/json.h>
#include <wait.h>
#include <ctime>

#include "host.h"
#include "util.h"


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

class timer {
public:
    boost::asio::deadline_timer scheduler;

    timer(boost::asio::io_service &io_service, std::map<std::string, std::string> &configuration)
            : scheduler(boost::asio::deadline_timer(io_service)),
              config(configuration) {}

    template<class T, class U>
    void schedule(const T &fun, U &param) {
        this->set_interval();
        scheduler.async_wait(boost::bind(fun, boost::asio::placeholders::error, this, config, boost::ref(param)));
    }

private:
    std::map<std::string, std::string> &config;

    void set_interval() {
        scheduler.expires_from_now(boost::posix_time::seconds(get_interval(config)));
    }
};

void get_available_hosts(const boost::system::error_code & /*e*/, timer *scheduler,
                         std::map<std::string, std::string> &config, std::vector<Host> &available_hosts);

namespace tcp_proxy {
    namespace ip = boost::asio::ip;

    class bridge : public boost::enable_shared_from_this<bridge> {
    public:
        typedef ip::tcp::socket socket_type;
        typedef boost::shared_ptr<bridge> ptr_type;

        explicit bridge(boost::asio::io_service &ios)
                : downstream_socket_(ios),
                  upstream_socket_(ios) {}

        socket_type downstream_socket_;
        socket_type upstream_socket_;

        socket_type &downstream_socket() {
            // Client socket
            return downstream_socket_;
        }

        socket_type &upstream_socket() {
            // Remote socket
            return upstream_socket_;
        }

        void start(const std::string &upstream_host, unsigned short upstream_port) {
            upstream_socket_.async_connect(
                    ip::tcp::endpoint(
                            boost::asio::ip::address::from_string(upstream_host),
                            upstream_port),
                    boost::bind(&bridge::handle_upstream_connect,
                                shared_from_this(),
                                boost::asio::placeholders::error));
        }

        void handle_upstream_connect(const boost::system::error_code &error) {
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

    private:

        // Read from remote server complete, send data to client
        void handle_upstream_read(const boost::system::error_code &error,
                                  const size_t &bytes_transferred) {
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

        void handle_downstream_write(const boost::system::error_code &error) {
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

        void handle_downstream_read(const boost::system::error_code &error,
                                    const size_t &bytes_transferred) {
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

        void handle_upstream_write(const boost::system::error_code &error) {
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

        void close() {
            boost::mutex::scoped_lock lock(mutex_);

            if (downstream_socket_.is_open()) {
                downstream_socket_.close();
            }

            if (upstream_socket_.is_open()) {
                upstream_socket_.close();
            }
        }

        enum {
            max_data_length = 8196 // 8KB
        };
        unsigned char downstream_data_[max_data_length];
        unsigned char upstream_data_[max_data_length];
        bool closed = false;

        boost::mutex mutex_;

    public:
        class acceptor {
        public:
            acceptor(boost::asio::io_service &io_service,
                     const std::string &local_host, unsigned short local_port,
                     timer &t,
                     std::map<std::string, std::string> &config)
                    : io_service_(io_service),
                      localhost_address(boost::asio::ip::address_v4::from_string(local_host)),
                      acceptor_(io_service_, ip::tcp::endpoint(localhost_address, local_port)),
                      signal_(io_service, SIGCHLD) {
                const boost::system::error_code null;
                get_available_hosts(null, &t, config, available_hosts_);
                wait_for_signal();
                accept_connections();
            }

            void wait_for_signal() {
                signal_.async_wait(boost::bind(&acceptor::handle_signal_wait, this));
            }

            void handle_signal_wait() {
                if (getpid() == ppid) {
                    int status = 0;
                    waitpid(-1, &status, WNOHANG > 0);
                    wait_for_signal();
                }
            }

            bool accept_connections() {
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

        private:
            void handle_accept(const boost::system::error_code &error) {
                if (!error) {
                    if (available_hosts_.empty()) {
                        std::string message("No hosts available to forward requests to.");
                        std::cout << message << std::endl;
                        std::string contentLength = std::to_string(message.length());
                        std::string response = "HTTP/1.1 503 Service Unavailable\nX-Oystr-Proxy: true\nContent-Type: text/html; charset=utf-8\nContent-Length: " + contentLength + "\n\n" + message;
                        session_->downstream_socket().write_some(boost::asio::buffer(response));
                        accept_connections();
                        return;
                    }
                    unsigned short index = random() % available_hosts_.size();
                    const std::string host(available_hosts_[index].host);
                    const unsigned short port(available_hosts_[index].port);

                    io_service_.notify_fork(boost::asio::io_service::fork_prepare);
                    auto pid = fork();
                    if (pid == 0) {
                        io_service_.notify_fork(boost::asio::io_service::fork_child);
                        std::cout << "[" << get_date("%d/%m/%Y %H:%M:%S") << "] " << session_->downstream_socket_.remote_endpoint() << " -> " << host << ":" << port << std::endl;

                        acceptor_.close();
                        signal_.cancel();
                        session_->start(host, port);
                    } else {
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

            boost::asio::io_service &io_service_;
            ip::address_v4 localhost_address;
            ip::tcp::acceptor acceptor_;
            ptr_type session_;
            pid_t ppid;
            boost::asio::signal_set signal_;
            std::vector<Host> available_hosts_;
        };
    };
}

// TODO:
// * Daemonize;
// * Log to /var/log/<name>.log;
// * Limit forking and reuse childs instead of killing them.
int main() {
    std::map<std::string, std::string> config;
    read_config(config);

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
        tcp_proxy::bridge::acceptor acceptor(ios, local_host, local_port, t, config);
        std::cout << "Listening on: " << local_host << ":" << local_port << "\n" << std::endl;

        ios.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

// TODO: Implement a HTTP Client.
void get_available_hosts(const boost::system::error_code & /*e*/, timer *scheduler,
                         std::map<std::string, std::string> &config, std::vector<Host> &available_hosts) {
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
                available_hosts.clear();
                for (auto item : data) {
                    const std::string host(item["host"].asString());
                    const unsigned short port(item["port"].asInt());
                    const Host hostObj(host, port);

                    available_hosts.push_back(hostObj);
                }
            } else {
                std::cout << "Non JSON response: " << http_data->c_str() << std::endl;
            }
        }
    }

    scheduler->schedule(get_available_hosts, available_hosts);
}
