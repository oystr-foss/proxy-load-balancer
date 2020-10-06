#include "util.h"
#include <fstream>
#include <algorithm>
#include <ctime>


int get_interval(std::map<std::string, std::string> &config) {
    if (config.count("refresh_interval") == 1) {
        std::string interval = config["refresh_interval"];
        try {
            int i = std::stoi(interval);
            return i > 0 ? i : 60;
        } catch (std::invalid_argument &e) {
            std::cerr << "refresh_interval must be a positive integer and greater than 0" << std::endl;
            exit(1);
        }
    }
    return 60;
}

void read_config(std::map<std::string, std::string> &config) {
    std::ifstream file("/etc/oplb/proxy.conf");
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

            if (line[0] == '#' || line.empty()) {
                continue;
            }

            auto delimiterPos = line.find('=');
            auto name = line.substr(0, delimiterPos);
            auto val = line.substr(delimiterPos + 1);

            config[name] = val;
        }
    } else {
        std::cerr << "proxy.conf not found" << std::endl;
        exit(1);
    }
}

std::string get_date() {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    char date[12];
    std::strftime(date, sizeof(date), "%d/%m/%Y", ltm);
    return std::string(date);
}

std::string get_date(const char *format) {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    char date[32]; // should we be concerned about this?
    std::strftime(date, sizeof(date), format, ltm);
    return std::string(date);
}