#include "util.hpp"
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

void read_config(std::map<std::string, std::string> &config, const std::string& config_file) {
    std::ifstream file(config_file);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

            if (line[0] == '#' || line.empty()) {
                continue;
            }

            auto delimiter_pos = line.find('=');
            auto name = line.substr(0, delimiter_pos);
            auto val = line.substr(delimiter_pos + 1);

            config[name] = val;
        }
    } else {
        std::cerr << "proxy.conf not found" << std::endl;
        exit(1);
    }
}

void read_config(std::map<std::string, std::string> &config) {
    read_config(config, "/etc/oplb/proxy.conf");
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