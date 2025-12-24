#include "osu/parser.hpp"
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <string_view>
#include <vector>

using namespace emscripten;

std::string get_property_wasm(uintptr_t data_ptr, int length, std::string key) {
    std::string_view content(reinterpret_cast<const char *>(data_ptr), length);
    return osu_parser::get_property(content, key);
}

val get_properties_wasm(uintptr_t data_ptr, int length, val keys) {
    std::string_view content(reinterpret_cast<const char *>(data_ptr), length);

    std::vector<std::string> key_list = vecFromJSArray<std::string>(keys);
    auto results = osu_parser::get_properties(content, key_list);

    val obj = val::object();
    for (const auto &key : key_list) {
        obj.set(key, results[key]);
    }

    return obj;
}

EMSCRIPTEN_BINDINGS(osu_parser) {
    function("get_property", &get_property_wasm);
    function("get_properties", &get_properties_wasm);
}

int main() { return 0; }
