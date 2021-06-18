#include <iostream>
#include <map>

#ifndef OLPB_UTIL_HPP
#define OLPB_UTIL_HPP

int get_interval(std::map<std::string, std::string> &config);
void read_config(std::map<std::string, std::string> &config, const std::string& config_file);
void read_config(std::map<std::string, std::string> &config);
std::string get_date();
std::string get_date(const char *format);

#endif //OLPB_UTIL_HPP
