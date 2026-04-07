#include "audio.hpp"

#ifdef OSU_PARSER_HAS_SNDFILE
#include <sndfile.h>
#endif

#include <filesystem>

double get_audio_duration_seconds(const std::string& beatmap_location, const std::string& audio_filename) {
#ifndef OSU_PARSER_HAS_SNDFILE
    (void)beatmap_location;
    (void)audio_filename;
    return 0.0;
#else
    if (beatmap_location.empty() || audio_filename.empty()) {
        return 0.0;
    }

    std::error_code err;
    const std::filesystem::path beatmap_path(beatmap_location);
    const std::filesystem::path audio_path = beatmap_path.parent_path() / std::filesystem::path(audio_filename);
    const std::filesystem::path normalized_audio_path = audio_path.lexically_normal();

    if (!std::filesystem::exists(normalized_audio_path, err) || err) {
        return 0.0;
    }

    SF_INFO info = {};
    SNDFILE* snd = sf_open(normalized_audio_path.string().c_str(), SFM_READ, &info);

    if (snd == nullptr) {
        return 0.0;
    }

    const double duration =
        info.frames > 0 && info.samplerate > 0 ? static_cast<double>(info.frames) / info.samplerate : 0.0;
    sf_close(snd);
    return duration;
#endif
}
