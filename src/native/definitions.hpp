#pragma once
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

enum OSU_SECTIONS { General = 0, Editor, Metadata, Difficulty, Events, TimingPoints, Colours, HitObjects };

struct general_section {
    std::string audio_filename;
    int audio_lead_in = 0;  // ms of silence before audio
    std::string audio_hash; // deprecated
    int preview_time = -1;  // preview start time in ms
    int countdown = 1;      // 0=none, 1=normal, 2=half, 3=double
    std::string sample_set = "Normal";
    double stack_leniency = 0.7; // how close objects stack (0-1)
    int mode = 0;                // 0=std, 1=taiko, 2=ctb, 3=mania
    int letterbox_in_breaks = 0;
    int story_fire_in_front = 1; // deprecated
    int use_skin_sprites = 0;
    int always_show_playfield = 0; // deprecated
    std::string overlay_position = "NoChange";
    std::string skin_preference;
    int epilepsy_warning = 0;
    int countdown_offset = 0;
    int special_style = 0; // mania n+1 layout
    int widescreen_storyboard = 0;
    int samples_match_playback_rate = 0;
};

struct editor_section {
    std::vector<int> bookmarks; // bookmark times in ms
    double distance_spacing = 1.0;
    int beat_divisor = 4;
    int grid_size = 4;
    double timeline_zoom = 1.0;
};

struct metadata_section {
    std::string title;
    std::string title_unicode;
    std::string artist;
    std::string artist_unicode;
    std::string creator;
    std::string version; // difficulty name
    std::string source;
    std::string tags;
    int beatmap_id = -1;
    int beatmap_set_id = -1;
};

struct difficulty_section {
    double hp_drain_rate = 5.0;      // 0-10
    double circle_size = 5.0;        // 0-10
    double overall_difficulty = 5.0; // 0-10
    double approach_rate = 5.0;      // 0-10
    double slider_multiplier = 1.4;  // base slider velocity
    double slider_tick_rate = 1.0;   // ticks per beat
};

struct event_background {
    std::string filename;
    int x_offset = 0;
    int y_offset = 0;
};

struct event_video {
    std::string filename;
    int start_time = 0;
    int x_offset = 0;
    int y_offset = 0;
};

struct event_break {
    int start_time = 0;
    int end_time = 0;
};

struct timing_point {
    int time = 0;
    double beat_length = 0.0;
    int meter = 4;      // beats per measure
    int sample_set = 0; // 0=default, 1=normal, 2=soft, 3=drum
    int sample_index = 0;
    int volume = 100;
    int uninherited = 1; // 1=red line (BPM), 0=green line (SV)
    int effects = 0;     // bit 0=kiai, bit 3=omit first barline
};

struct colour_section {
    std::vector<std::array<int, 3>> combos;
    std::optional<std::array<int, 3>> slider_track_override;
    std::optional<std::array<int, 3>> slider_border;
};

struct hit_sample {
    int normal_set = 0;
    int addition_set = 0;
    int index = 0;
    int volume = 0;
    std::string filename;
};

struct hit_object {
    int x = 0;    // 0-512 (playfield coords)
    int y = 0;    // 0-384
    int time = 0; // ms
    int type = 0; // bitmask
    int hit_sound = 0;
    hit_sample sample;

    char curve_type = 'L'; // B=bezier, C=catmull, L=linear, P=perfect circle
    std::vector<std::pair<int, int>> curve_points;
    int slides = 1;      // repeat count + 1
    double length = 0.0; // visual length in osu pixels
    std::vector<int> edge_sounds;
    std::vector<std::pair<int, int>> edge_sets;

    int end_time = 0; // spinner/hold end time
};

struct osu_file_format {
    int version = 14;
    general_section general;
    editor_section editor;
    metadata_section metadata;
    difficulty_section difficulty;
    std::optional<event_background> background;
    std::optional<event_video> video;
    std::vector<event_break> breaks;
    std::vector<timing_point> timing_points;
    colour_section colours;
    std::vector<hit_object> hit_objects;
};

inline const std::unordered_map<std::string, std::string>& get_key_to_section() {
    static const std::unordered_map<std::string, std::string> map = {
        {"AudioFilename", "[General]"},
        {"AudioLeadIn", "[General]"},
        {"AudioHash", "[General]"},
        {"PreviewTime", "[General]"},
        {"Countdown", "[General]"},
        {"SampleSet", "[General]"},
        {"StackLeniency", "[General]"},
        {"Mode", "[General]"},
        {"LetterboxInBreaks", "[General]"},
        {"StoryFireInFront", "[General]"},
        {"UseSkinSprites", "[General]"},
        {"AlwaysShowPlayfield", "[General]"},
        {"OverlayPosition", "[General]"},
        {"SkinPreference", "[General]"},
        {"EpilepsyWarning", "[General]"},
        {"CountdownOffset", "[General]"},
        {"SpecialStyle", "[General]"},
        {"WidescreenStoryboard", "[General]"},
        {"SamplesMatchPlaybackRate", "[General]"},
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

inline const std::unordered_set<std::string>& get_special_keys() {
    static const std::unordered_set<std::string> set{"Background", "Video", "Storyboard"};
    return set;
}
