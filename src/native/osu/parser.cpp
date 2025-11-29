#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include "./parser.hpp"
#include "./definitions.hpp"

const std::unordered_set<std::string_view> VIDEO_EXTENSIONS = {
    ".mp4", ".avi", ".flv", ".mov", ".wmv", ".m4v", ".mpg", ".mpeg"
};

const std::unordered_set<std::string_view> IMAGE_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".bmp", ".gif"
};

std::string_view trim_view(std::string_view s) {
    size_t start = s.find_first_not_of(" \t\r\n");

    if (start == std::string_view::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string trim(const std::string &s) {
    std::string_view result = trim_view(s);
    return std::string(result);
}

std::vector<std::string_view> split_view(std::string_view s, char delim) {
    std::vector<std::string_view> result;

    size_t start = 0;
    size_t end = s.find(delim);
    
    while (end != std::string_view::npos) {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delim, start);
    }
    result.push_back(s.substr(start));
    
    return result;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

std::ifstream read_file(std::string &location) {
#ifdef _WIN32
    std::wstring wide_path = utf8_to_wide(location);
    std::ifstream file(wide_path, std::ios::binary);
    return file;
#else
    std::ifstream file(location);
    return file;
#endif
}

std::string normalize_path(const std::string &path) {
#ifdef _WIN32
    std::string normalized = path;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    return normalized;
#else
    return path;
#endif
}

std::string_view get_extension(std::string_view filename) {
    size_t dot_pos = filename.find_last_of('.');

    if (dot_pos == std::string_view::npos) {
        return "";
    }

    return filename.substr(dot_pos);
}

std::optional<std::string> get_special_key(std::string_view key, std::string_view line) {
    if (key == "Background" || key == "Video") {
        std::vector<std::string_view> parts = split_view(line, ',');

        if (parts.size() < 3) {
            return std::nullopt;
        }
        
        std::string_view event_type = trim_view(parts[0]);
        std::string_view start_time = trim_view(parts[1]);

        if (event_type != "0" || start_time != "0") {
            return std::nullopt;
        }
        
        std::string_view filename = trim_view(parts[2]);
        
        // remove quotes if present
        if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
            filename = filename.substr(1, filename.size() - 2);
        }
        
        std::string_view ext = get_extension(filename);
        std::string ext_lower(ext);
        std::transform(ext_lower.begin(), ext_lower.end(), ext_lower.begin(), ::tolower);
        
        if (key == "Video") {
            if (VIDEO_EXTENSIONS.count(ext_lower) == 0) {
                return std::nullopt;
            }
        } else {
            if (IMAGE_EXTENSIONS.count(ext_lower) == 0) {
                return std::nullopt;
            }
        }
        
        return normalize_path(std::string(filename));
    } else if (key == "Storyboard") {
        // TODO:
        return std::nullopt;
    }
    
    return std::nullopt;
}

std::string osu_parser::get_property(std::string &location, std::string &key) {
    auto file = read_file(location);

    if (!file.is_open()) {
        std::cout << "failed to open: " << location << "\n";
        return "";
    }
    
    bool is_special_key = SPECIAL_KEYS.count(key) > 0;

    std::string current_section;
    std::string line;
    std::string special_section;
    
    if (is_special_key) {
        if (KEY_TO_SECTION.find(key) == KEY_TO_SECTION.end()) {
            std::cout << "failed to find special key: " << key << "\n";
            return "";
        }
        special_section = KEY_TO_SECTION.at(key);
    }
    
    while (getline(file, line)) {
        std::string_view line_view = trim_view(line);
        
        if (line_view.empty() || line_view[0] == '/') {
            continue;
        }
        
        if (line_view[0] == '[') {
            current_section = std::string(line_view);
            continue;
        }
        
        if (is_special_key) {
            if (special_section != current_section) {
                continue;
            }
            
            auto result = get_special_key(key, line_view);

            if (result.has_value()) {
                return result.value();
            }

            continue;
        }
        
        size_t delimiter_i = line_view.find(':');

        if (delimiter_i == std::string_view::npos) {
            continue;
        }
        
        std::string_view current_key = trim_view(line_view.substr(0, delimiter_i));
        
        if (current_key == key) {
            std::string_view value = trim_view(line_view.substr(delimiter_i + 1));
            return std::string(value);
        }
    }
    
    return "";
}

std::vector<std::string> osu_parser::get_section() {
    std::vector<std::string> result;
    return result;
}