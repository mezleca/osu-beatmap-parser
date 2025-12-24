#pragma once
#include <napi.h>

class ParserAddon : public Napi::Addon<ParserAddon> {
public:
    ParserAddon(Napi::Env env, Napi::Object exports) {
        DefineAddon(
            exports,
            {
                InstanceMethod("get_property", &ParserAddon::get_property),
                InstanceMethod("get_properties", &ParserAddon::get_properties),
            });
    }

    Napi::Value get_property(const Napi::CallbackInfo &info);
    Napi::Value get_properties(const Napi::CallbackInfo &info);
};
