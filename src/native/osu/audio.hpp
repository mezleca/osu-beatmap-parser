#pragma once
#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>

class AudioAnalyzer
{
private:
    std::unordered_map<std::string, double> cache;
    mutable std::mutex cache_mutex;
    
    bool get_cache(const std::string &key, double &duration) const {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(key);
        if (it != cache.end())
        {
            duration = it->second;
            return true;
        }
        return false;
    }

    void set_cache(const std::string &key, double duration) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache[key] = duration;
    }

    void clear_cache() {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cache.clear();
    }
public:
    double get_audio_duration(std::filesystem::path &location);
};