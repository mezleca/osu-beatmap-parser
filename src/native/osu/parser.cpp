#include "./parser.hpp"
#include "../definitions.hpp"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>

const std::unordered_set<std::string> VIDEO_EXTENSIONS = {".mp4", ".avi", ".flv", ".mov",
                                                          ".wmv", ".m4v", ".mpg", ".mpeg"};

const std::unordered_set<std::string> IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".gif"};

std::string_view trim_view(std::string_view s) {
    size_t start = s.find_first_not_of(" \t\r\n");

    if (start == std::string_view::npos) {
        return "";
    }

    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string trim(const std::string& s) {
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

std::string normalize_path(const std::string& path) {
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

std::string osu_parser::get_property(std::string_view content, std::string_view key) {
    bool is_special_key = SPECIAL_KEYS.count(std::string(key)) > 0;
    std::string current_section;
    std::string special_section;

    if (is_special_key) {
        std::string key_str(key);
        if (KEY_TO_SECTION.find(key_str) == KEY_TO_SECTION.end()) {
            std::cout << "failed to find special key: " << key << "\n";
            return "";
        }
        special_section = KEY_TO_SECTION.at(key_str);
    }

    size_t start = 0;
    size_t end = content.find('\n');

    while (end != std::string_view::npos) {
        std::string_view line_view = trim_view(content.substr(start, end - start));
        start = end + 1;
        end = content.find('\n', start);

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

    // check last line if no newline at end
    if (start < content.size()) {
        std::string_view line_view = trim_view(content.substr(start));
        if (line_view.empty() || line_view[0] == '/') {
            return "";
        }

        // section change at end
        if (line_view[0] == '[') {
            return "";
        }

        if (is_special_key) {
            if (special_section == current_section) {
                auto result = get_special_key(key, line_view);
                if (result.has_value()) {
                    return result.value();
                }
            }
            return "";
        }

        size_t delimiter_i = line_view.find(':');

        if (delimiter_i != std::string_view::npos) {
            std::string_view current_key = trim_view(line_view.substr(0, delimiter_i));
            if (current_key == key) {
                std::string_view value = trim_view(line_view.substr(delimiter_i + 1));
                return std::string(value);
            }
        }
    }

    return "";
}

std::unordered_map<std::string, std::string>
osu_parser::get_properties(std::string_view content, const std::vector<std::string>& keys) {
    std::unordered_map<std::string, std::string> results;
    std::unordered_map<std::string_view, std::string_view> key_to_section_map;
    std::unordered_set<std::string_view> keys_to_find;

    for (const auto& key : keys) {
        keys_to_find.insert(key);
        if (KEY_TO_SECTION.count(key)) {
            key_to_section_map[key] = KEY_TO_SECTION.at(key);
        }
    }

    std::string current_section;
    size_t start = 0;
    size_t end = content.find('\n');

    auto process_line = [&](std::string_view line_view) {
        if (line_view.empty() || line_view[0] == '/') {
            return;
        }

        if (line_view[0] == '[') {
            current_section = std::string(line_view);
            return;
        }

        size_t delimiter_i = line_view.find(':');
        if (delimiter_i != std::string_view::npos) {
            std::string_view current_key = trim_view(line_view.substr(0, delimiter_i));

            if (keys_to_find.count(current_key)) {
                bool correct_section = !key_to_section_map.count(current_key) ||
                                       key_to_section_map.at(current_key) == current_section;

                bool not_found_yet = results.find(std::string(current_key)) == results.end();

                if (correct_section && not_found_yet) {
                    std::string_view value = trim_view(line_view.substr(delimiter_i + 1));
                    results[std::string(current_key)] = std::string(value);
                }
            }
        }

        for (const auto& key : keys) {
            if (!SPECIAL_KEYS.count(key)) {
                continue;
            }

            if (results.find(key) != results.end()) {
                continue;
            }

            if (key_to_section_map.count(key) && key_to_section_map.at(key) == current_section) {
                auto result = get_special_key(key, line_view);
                if (result.has_value()) {
                    results[key] = result.value();
                }
            }
        }
    };

    while (end != std::string_view::npos) {
        std::string_view line_view = trim_view(content.substr(start, end - start));
        process_line(line_view);
        start = end + 1;
        end = content.find('\n', start);
    }

    if (start < content.size()) {
        std::string_view line_view = trim_view(content.substr(start));
        process_line(line_view);
    }

    return results;
}

std::vector<std::string> osu_parser::get_section() {
    std::vector<std::string> result;
    return result;
}

std::map<std::string, std::string> osu_parser::parse(std::string_view content) { return {}; }
