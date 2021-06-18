//
// Created by realngnx on 11/06/2021.
//

#ifndef OPLB_TIMER_HPP
#define OPLB_TIMER_HPP

#include <cstdlib>
#include <map>
#include <boost/asio.hpp>
#include "sharding/node.hpp"
#include "sharding/sharder.hpp"


class timer {
public:
    boost::asio::deadline_timer scheduler;

    timer(boost::asio::io_service &io_service, std::map<std::string, std::string> &configuration);

    void schedule(void (&fun)(const boost::system::error_code & /*e*/, timer *scheduler,
                              std::map<std::string, std::string> &config, ConsistentHash& sharder), ConsistentHash& param);

private:
    std::map<std::string, std::string> &config;

    void set_interval();
};


#endif //OPLB_TIMER_HPP
