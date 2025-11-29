#include <iostream>
#include <sndfile.h>
#include "audio.hpp"

double AudioAnalyzer::get_audio_duration(std::filesystem::path &location) {
    double duration = 0.0;
    
    if (get_cache(location, duration)) {
        return duration;
    }

    if (!std::filesystem::exists(location)) {
        std::cout << location << " does not exists\n";
        return 0.0;
    }

    SF_INFO sfinfo{};
    SNDFILE *file = sf_open(location.c_str(), SFM_READ, &sfinfo);

    if (!file) {
        return 0.0;
    }

    duration = static_cast<double>(sfinfo.frames) / sfinfo.samplerate;
    
    sf_close(file);
    set_cache(location, duration);
    return duration;
}