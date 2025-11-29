#pragma once
#include <string>
#include <vector>

namespace osu_parser {
    void read(std::string location);
    std::string get_property(std::string &location, std::string &key);
    std::vector<std::string> get_section();
};