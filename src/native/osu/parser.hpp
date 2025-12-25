#pragma once
#include "../definitions.hpp"
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace osu_parser {

std::string get_property(std::string_view content, std::string_view key);

std::unordered_map<std::string, std::string> get_properties(std::string_view content,
                                                            const std::vector<std::string>& keys);

// returns raw section content as vector of lines
std::vector<std::string> get_section(std::string_view content, std::string_view section_name);

// returns complete parsed file
osu_file_format parse(std::string_view content);

}; // namespace osu_parser
