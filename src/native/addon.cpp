#include <filesystem>
#include <napi.h>
#include <iostream>
#include <sndfile.h>
#include <string>
#include <thread>
#include <fstream>
#include <vector>
#include <atomic>
#include "osu/parser.hpp"
#include "osu/audio.hpp"
#include "addon.hpp"
#include "pool.hpp"

#define NOOP_FUNC(env) Napi::Function::New(env, [](const Napi::CallbackInfo& info){})

AudioAnalyzer audio_analizer;

std::filesystem::path dirname(const std::string &path) {
    std::filesystem::path p(path);
    return p.parent_path();
}

std::string read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return "";
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string buffer(size, ' ');

    if (file.read(buffer.data(), size)) {
        return buffer;
    }

    return "";
}

Napi::Value ParserAddon::get_property(const Napi::CallbackInfo& info) {
    if (info.Length() < 2) {
        return Napi::String::New(info.Env(), "");
    }

    if (!info[0].IsString() || !info[1].IsString()) {
        return Napi::String::New(info.Env(), "");
    }

    std::string location = info[0].As<Napi::String>().Utf8Value();
    std::string key = info[1].As<Napi::String>().Utf8Value();
    std::string content = read_file(location);

    if (content.empty()) {
        return Napi::String::New(info.Env(), "");
    }

    return Napi::String::From(info.Env(), osu_parser::get_property(content, key));
}

Napi::Value ParserAddon::get_properties(const Napi::CallbackInfo& info) {
    if (info.Length() < 2) {
        return Napi::Object::New(info.Env());
    }

    std::string location = info[0].As<Napi::String>().Utf8Value();
    Napi::Array keys_array = info[1].As<Napi::Array>();
    
    std::vector<std::string> keys;

    for (uint32_t i = 0; i < keys_array.Length(); i++) {
        Napi::Value val = keys_array[i];
        if (val.IsString()) {
            keys.push_back(val.As<Napi::String>().Utf8Value());
        }
    }

    std::string content = read_file(location);

    if (content.empty()) {
        return Napi::Object::New(info.Env());
    }

    auto results = osu_parser::get_properties(content, keys);

    Napi::Object obj = Napi::Object::New(info.Env());

    for (const auto& [k, v] : results) {
        obj.Set(k, v);
    }
    
    return obj;
}

struct BatchContext {
    std::vector<std::string> paths;
    std::vector<std::string> keys;
    std::vector<std::unordered_map<std::string, std::string>> results;
    std::atomic<size_t> completed_count{0};
    Napi::ThreadSafeFunction tsfn;
    Napi::Promise::Deferred deferred;
    bool needs_duration = false;
    
    BatchContext(Napi::Env env) : deferred(Napi::Promise::Deferred::New(env)) {}
};

void process_chunk(BatchContext* context, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        std::string content = read_file(context->paths[i]);
        if (!content.empty()) {
            context->results[i] = osu_parser::get_properties(content, context->keys);
            
            if (context->needs_duration) {
                std::string audio_file_name = osu_parser::get_property(content, "AudioFilename");
                if (!audio_file_name.empty()) {
                    std::filesystem::path audio_path = dirname(context->paths[i]) / audio_file_name;
                    double duration = audio_analizer.get_audio_duration(audio_path.string());
                    context->results[i]["Duration"] = std::to_string(duration);
                }
            }
        }
    }
    
    size_t completed = context->completed_count.fetch_add(end - start) + (end - start);
    
    if (completed == context->paths.size()) {
        auto callback = [context](Napi::Env env, Napi::Function jsCallback) {
            Napi::Array result_array = Napi::Array::New(env, context->results.size());
            
            for (size_t i = 0; i < context->results.size(); i++) {
                Napi::Object obj = Napi::Object::New(env);
                for (const auto& [k, v] : context->results[i]) {
                    obj.Set(k, v);
                }
                result_array[i] = obj;
            }
            
            context->deferred.Resolve(result_array);
        };
        
        context->tsfn.BlockingCall(callback);
        context->tsfn.Release();
    }
}

Napi::Value ParserAddon::process_beatmaps(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2) {
        auto deferred = Napi::Promise::Deferred::New(env);
        deferred.Reject(Napi::String::New(env, "Invalid arguments"));
        return deferred.Promise();
    }

    Napi::Array paths_array = info[0].As<Napi::Array>();
    Napi::Array keys_array = info[1].As<Napi::Array>();

    auto context = new BatchContext(env);
    
    for (uint32_t i = 0; i < paths_array.Length(); i++) {
        context->paths.push_back(Napi::Value(paths_array[i]).As<Napi::String>().Utf8Value());
    }
    
    for (uint32_t i = 0; i < keys_array.Length(); i++) {
        std::string key = Napi::Value(keys_array[i]).As<Napi::String>().Utf8Value();
        context->keys.push_back(key);
        if (key == "Duration") {
            context->needs_duration = true;
        }
    }
    
    context->results.resize(context->paths.size());
    
    context->tsfn = Napi::ThreadSafeFunction::New(
        env,
        NOOP_FUNC(env),
        "ProcessBeatmaps",
        0,
        1,
        [context](Napi::Env) {
            delete context;
        }
    );

    size_t total_files = context->paths.size();
    size_t thread_count = std::thread::hardware_concurrency();

    if (thread_count == 0) {
        thread_count = 1;
    }
    
    size_t chunk_size = (total_files + thread_count - 1) / thread_count;
    
    for (size_t t = 0; t < thread_count; t++) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, total_files);
        
        if (start >= end) {
            break;
        }

        pool.enqueue([context, start, end]() {
            process_chunk(context, start, end);
        });
    }

    return context->deferred.Promise();
}

Napi::Value ParserAddon::get_duration(const Napi::CallbackInfo& info) {
    if (info.Length() < 1) {
        return Napi::Number::New(info.Env(), 0.0);
    }

    if (!info[0].IsString()) {
        return Napi::String::New(info.Env(), "");
    }

    std::string location = info[0].As<Napi::String>().Utf8Value();
    std::string content = read_file(location);

    if (content.empty()) {
        return Napi::Number::New(info.Env(), 0.0);
    }

    std::string audio_file_name = osu_parser::get_property(content, "AudioFilename");
    std::filesystem::path audio_path = dirname(location) / audio_file_name;

    return Napi::Number::From(info.Env(), audio_analizer.get_audio_duration(audio_path.string()));
}

Napi::Value ParserAddon::get_audio_duration(const Napi::CallbackInfo& info) {
    if (info.Length() < 1) {
        return Napi::Number::New(info.Env(), 0.0);
    }

    if (!info[0].IsString()) {
        return Napi::String::New(info.Env(), "");
    }

    std::string location = info[0].As<Napi::String>().Utf8Value();
    std::filesystem::path audio_path(location);

    return Napi::Number::From(info.Env(), audio_analizer.get_audio_duration(audio_path.string()));
}

Napi::Value ParserAddon::test_promise(const Napi::CallbackInfo& info) {
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

NODE_API_ADDON(ParserAddon)