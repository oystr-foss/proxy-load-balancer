#include <iostream>
#include <map>

#ifndef OLPB_UTIL_H
#define OLPB_UTIL_H

int get_interval(std::map<std::string, std::string> &config);

void read_config(std::map<std::string, std::string> &config);

std::string get_date();

std::string get_date(const char *format);

#endif //OLPB_UTIL_H
