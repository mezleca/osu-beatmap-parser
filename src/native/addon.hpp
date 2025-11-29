#pragma once

#include <napi.h>
#include "pool.hpp"

class ParserAddon : public Napi::Addon<ParserAddon> {
public:
    ParserAddon(Napi::Env env, Napi::Object exports) {
        pool.initialize(std::thread::hardware_concurrency());
        DefineAddon(exports, {
            InstanceMethod("get_property", &ParserAddon::get_property),
            InstanceMethod("get_properties", &ParserAddon::get_properties),
            InstanceMethod("process_beatmaps", &ParserAddon::process_beatmaps),
            InstanceMethod("get_duration", &ParserAddon::get_duration),
            InstanceMethod("get_audio_duration", &ParserAddon::get_audio_duration),
            InstanceMethod("test_promise", &ParserAddon::test_promise)
        });
    }

    Napi::Value get_property(const Napi::CallbackInfo& info);
    Napi::Value get_properties(const Napi::CallbackInfo& info);
    Napi::Value process_beatmaps(const Napi::CallbackInfo& info);
    Napi::Value get_duration(const Napi::CallbackInfo& info);
    Napi::Value get_audio_duration(const Napi::CallbackInfo& info);
    Napi::Value test_promise(const Napi::CallbackInfo& info);
};