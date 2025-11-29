#pragma once
#include <napi.h>

class ProcessorAddon : public Napi::Addon<ProcessorAddon> {
public:
    ProcessorAddon(Napi::Env env, Napi::Object exports) {
        DefineAddon(exports, {
            InstanceMethod("get_property", &ProcessorAddon::get_property),
            InstanceMethod("test_promise", &ProcessorAddon::test_promise)
        });
    }

private:
    Napi::Value get_property(const Napi::CallbackInfo& info);
    Napi::Value test_promise(const Napi::CallbackInfo& info);
};