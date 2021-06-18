//
// Created by realngnx on 11/06/2021.
//

#include "timer.hpp"
#include "util.hpp"
#include <boost/bind.hpp>


timer::timer(boost::asio::io_service &io_service, std::map<std::string, std::string> &configuration):
    scheduler(boost::asio::deadline_timer(io_service)),
    config(configuration) {}

void timer::schedule(void (&fun)(const boost::system::error_code & /*e*/, timer *scheduler,
                                 std::map<std::string, std::string> &config, ConsistentHash& sharder), ConsistentHash& param) {
    this->set_interval();
    this->scheduler.async_wait(boost::bind(fun, boost::asio::placeholders::error, this, config, boost::ref(param)));
}

void timer::set_interval() {
    scheduler.expires_from_now(boost::posix_time::seconds(get_interval(config)));
}
