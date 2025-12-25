#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

enum OSU_SECTIONS {
  General = 0,
  Editor,
  Metadata,
  Difficulty,
  Events,
  TimingPoints,
  Colours,
  HitObjects
};

inline const std::unordered_map<std::string, std::string> & get_key_to_section() {
    static const std::unordered_map<std::string, std::string> map = {
        {"AudioFilename", "[General]"},
        {"AudioLeadIn", "[General]"},
        {"PreviewTime", "[General]"},
        {"Countdown", "[General]"},
        {"SampleSet", "[General]"},
        {"StackLeniency", "[General]"},
        {"Mode", "[General]"},
        {"LetterboxInBreaks", "[General]"},
        {"WidescreenStoryboard", "[General]"},
        {"Bookmarks", "[Editor]"},
        {"DistanceSpacing", "[Editor]"},
        {"BeatDivisor", "[Editor]"},
        {"GridSize", "[Editor]"},
        {"TimelineZoom", "[Editor]"},
        {"Title", "[Metadata]"},
        {"TitleUnicode", "[Metadata]"},
        {"Artist", "[Metadata]"},
        {"ArtistUnicode", "[Metadata]"},
        {"Creator", "[Metadata]"},
        {"Version", "[Metadata]"},
        {"Source", "[Metadata]"},
        {"Tags", "[Metadata]"},
        {"BeatmapID", "[Metadata]"},
        {"BeatmapSetID", "[Metadata]"},
        {"HPDrainRate", "[Difficulty]"},
        {"CircleSize", "[Difficulty]"},
        {"OverallDifficulty", "[Difficulty]"},
        {"ApproachRate", "[Difficulty]"},
        {"SliderMultiplier", "[Difficulty]"},
        {"SliderTickRate", "[Difficulty]"},
        {"Background", "[Events]"},
        {"Video", "[Events]"},
        {"Storyboard", "[Events]"},
    };
    return map;
}

inline const std::unordered_set<std::string> &get_special_keys() {
  static const std::unordered_set<std::string> set{"Background", "Video", "Storyboard"};
  return set;
}
