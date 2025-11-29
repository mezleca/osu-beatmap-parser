#include <napi.h>
#include <iostream>
#include <sndfile.h>
#include <thread>
#include "addon.hpp"
#include "osu/parser.hpp"

#define NOOP_FUNC(env) Napi::Function::New(env, [](const Napi::CallbackInfo& info){})

Napi::Value ProcessorAddon::get_property(const Napi::CallbackInfo& info) {
    if (info.Length() < 2) {
        return Napi::String::New(info.Env(), "");
    }

    if (!info[0].IsString() || !info[1].IsString()) {
        return Napi::String::New(info.Env(), "");
    }

    std::string location = info[0].As<Napi::String>().Utf8Value();
    std::string key = info[1].As<Napi::String>().Utf8Value();

    return Napi::String::From(info.Env(), osu_parser::get_property(location, key));
}

Napi::Value ProcessorAddon::test_promise(const Napi::CallbackInfo& info) {
    auto deffered = Napi::Promise::Deferred::New(info.Env());
    auto tsfn = Napi::ThreadSafeFunction::New(info.Env(), NOOP_FUNC(info.Env()), "tsfn", 0, 1);

    std::thread([tsfn, deffered]() {
        tsfn.NonBlockingCall([deffered](Napi::Env env, Napi::Function) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            deffered.Resolve(Napi::Boolean::New(env, true));
        });

        tsfn.Release();
    }).detach();

    return deffered.Promise();
}

NODE_API_ADDON(ProcessorAddon)