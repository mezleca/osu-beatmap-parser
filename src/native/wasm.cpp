#include "osu/parser.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>

using namespace emscripten;

std::string get_property_wasm(uintptr_t data_ptr, int length, std::string key) {
    std::string_view content(reinterpret_cast<const char*>(data_ptr), length);
    return osu_parser::get_property(content, key);
}

val get_properties_wasm(uintptr_t data_ptr, int length, val keys) {
    std::string_view content(reinterpret_cast<const char*>(data_ptr), length);

    std::vector<std::string> keys_vec;
    const int keys_len = keys["length"].as<int>();
    for (int i = 0; i < keys_len; ++i) {
        keys_vec.push_back(keys[i].as<std::string>());
    }

    auto results = osu_parser::get_properties(content, keys_vec);

    val obj = val::object();
    for (const auto& [k, v] : results) {
        obj.set(k, v);
    }

    return obj;
}

EMSCRIPTEN_BINDINGS(osu_parser) {
    function("get_property", &get_property_wasm);
    function("get_properties", &get_properties_wasm);
}
