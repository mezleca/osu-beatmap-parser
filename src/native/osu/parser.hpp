#pragma once
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace osu_parser {
    std::string get_property(std::string_view content, std::string_view key);
    std::unordered_map<std::string, std::string> get_properties(std::string_view content,
                                                                const std::vector<std::string>& keys);

    // TODO:
    std::vector<std::string> get_section();
    std::map<std::string, std::string> parse(std::string_view content);
};
