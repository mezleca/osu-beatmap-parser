#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>

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

const std::unordered_map<std::string, std::string> KEY_TO_SECTION = {
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
    {"Storyboard", "[Events]"}
};

const std::unordered_set<std::string> SPECIAL_KEYS {"Background", "Video", "Storyboard"};