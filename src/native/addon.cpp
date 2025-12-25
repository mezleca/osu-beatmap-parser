#include "osu/parser.hpp"
#include <napi.h>
#include <string>

Napi::Value get_property(const Napi::CallbackInfo &info) {
    if (info.Length() < 2) {
        return Napi::String::New(info.Env(), "");
    }

    if (!info[0].IsBuffer() || !info[1].IsString()) {
        return Napi::String::New(info.Env(), "");
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    std::string key = info[1].As<Napi::String>().Utf8Value();
    std::string_view content(buffer.Data(), buffer.Length());

    return Napi::String::New(info.Env(), osu_parser::get_property(content, key));
}

Napi::Value get_properties(const Napi::CallbackInfo &info) {
    if (info.Length() < 2) {
        return Napi::Object::New(info.Env());
    }

    if (!info[0].IsBuffer() || !info[1].IsArray()) {
        return Napi::Object::New(info.Env());
    }

    auto buffer = info[0].As<Napi::Buffer<char>>();
    Napi::Array keys_array = info[1].As<Napi::Array>();
    std::vector<std::string> keys;

    for (uint32_t i = 0; i < keys_array.Length(); i++) {
        Napi::Value val = keys_array[i];
        if (val.IsString()) {
        keys.push_back(val.As<Napi::String>().Utf8Value());
        }
    }

    std::string_view content(buffer.Data(), buffer.Length());
    auto results = osu_parser::get_properties(content, keys);

    Napi::Object obj = Napi::Object::New(info.Env());

    for (const auto& [k, v] : results) {
        obj.Set(k, v);
    }

    return obj;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "get_property"),
              Napi::Function::New(env, get_property));
  exports.Set(Napi::String::New(env, "get_properties"),
              Napi::Function::New(env, get_properties));
  return exports;
}

NODE_API_MODULE(osu_beatmap_parser, Init)
