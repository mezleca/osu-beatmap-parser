#include "filter.hpp"

#include <algorithm>
#include <chrono>
#include <charconv>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <optional>
#include <unordered_map>

#define OSU_QUERY_FIELD_LIST(X)                                                                                        \
    X(std::string_view, text, artist)                                                                                  \
    X(std::string_view, text, creator, "author", "mapper")                                                             \
    X(std::string_view, text, title)                                                                                   \
    X(std::string_view, text, difficulty, "diff")                                                                      \
    X(double, number, ar)                                                                                              \
    X(double, number, cs)                                                                                              \
    X(double, number, od)                                                                                              \
    X(double, number, hp, "dr")                                                                                        \
    X(double, number, keys, "key")                                                                                     \
    X(double, number, star, "stars", "sr")                                                                             \
    X(double, number, bpm)                                                                                             \
    X(double, number, length)                                                                                          \
    X(double, number, drain)                                                                                           \
    X(int32_t, enum_list, mode)                                                                                        \
    X(int32_t, enum_list, status)                                                                                      \
    X(double, number, played)                                                                                          \
    X(bool, flag, unplayed)                                                                                            \
    X(double, number, speed)                                                                                           \
    X(std::string_view, text, source)                                                                                  \
    X(std::string_view, text, tags, "tag")

#define OSU_QUERY_OPERATOR_LIST(X)                                                                                     \
    X("!=", ne)                                                                                                        \
    X("!:", not_contains)                                                                                              \
    X("!~", not_contains)                                                                                              \
    X(">=", gte)                                                                                                       \
    X("<=", lte)                                                                                                       \
    X("==", eq)                                                                                                        \
    X("^=", starts_with)                                                                                               \
    X("$=", ends_with)                                                                                                 \
    X("~=", contains)                                                                                                  \
    X("=", eq)                                                                                                         \
    X(":", contains)                                                                                                   \
    X(">", gt)                                                                                                         \
    X("<", lt)

#define OSU_TEXT_QUERY_ACCESSORS(X)                                                                                    \
    X(artist, beatmap.artist)                                                                                          \
    X(creator, beatmap.creator)                                                                                        \
    X(title, beatmap.title)                                                                                            \
    X(difficulty, beatmap.difficulty)                                                                                  \
    X(source, beatmap.source)                                                                                          \
    X(tags, beatmap.tags)

#define OSU_NUMBER_QUERY_ACCESSORS(X)                                                                                  \
    X(ar, beatmap.approach_rate)                                                                                       \
    X(cs, beatmap.circle_size)                                                                                         \
    X(od, beatmap.overall_difficulty)                                                                                  \
    X(hp, beatmap.hp_drain)                                                                                            \
    X(keys, beatmap.circle_size)                                                                                       \
    X(drain, static_cast<double>(beatmap.drain_time))                                                                  \
    X(speed, static_cast<double>(beatmap.mania_scroll_speed))                                                          \
    X(star, get_star_rating_for_mode(beatmap))                                                                         \
    X(bpm, get_common_bpm(beatmap.timing_points, beatmap.total_time))                                                  \
    X(length, static_cast<double>(beatmap.total_time) / 1000.0)

#define OSU_TEXT_SORT_ACCESSORS(X)                                                                                     \
    X(artist, to_lower_copy(beatmap.artist))                                                                           \
    X(title, to_lower_copy(beatmap.title))                                                                             \
    X(creator, to_lower_copy(beatmap.creator))                                                                         \
    X(difficulty, to_lower_copy(beatmap.difficulty))                                                                   \
    X(source, to_lower_copy(beatmap.source))                                                                           \
    X(tags, to_lower_copy(beatmap.tags))                                                                               \
    X(folder_name, to_lower_copy(beatmap.folder_name))                                                                 \
    X(audio_file_name, to_lower_copy(beatmap.audio_file_name))                                                         \
    X(osu_file_name, to_lower_copy(beatmap.osu_file_name))

#define OSU_NUMBER_SORT_ACCESSORS(X)                                                                                   \
    X(star, get_star_rating_for_mode(beatmap))                                                                         \
    X(bpm, get_common_bpm(beatmap.timing_points, beatmap.total_time))                                                  \
    X(ar, beatmap.approach_rate)                                                                                       \
    X(cs, beatmap.circle_size)                                                                                         \
    X(od, beatmap.overall_difficulty)                                                                                  \
    X(hp, beatmap.hp_drain)                                                                                            \
    X(length, static_cast<double>(beatmap.total_time) / 1000.0)                                                        \
    X(drain_time, static_cast<double>(beatmap.drain_time))                                                             \
    X(total_time, static_cast<double>(beatmap.total_time))                                                             \
    X(audio_preview_time, static_cast<double>(beatmap.audio_preview_time))                                             \
    X(mode, static_cast<double>(beatmap.mode))                                                                         \
    X(ranked_status, static_cast<double>(beatmap.ranked_status))                                                       \
    X(beatmap_id, static_cast<double>(beatmap.beatmap_id))                                                             \
    X(difficulty_id, static_cast<double>(beatmap.difficulty_id))                                                       \
    X(thread_id, static_cast<double>(beatmap.thread_id))                                                               \
    X(last_played, static_cast<double>(beatmap.last_played))                                                           \
    X(last_checked, static_cast<double>(beatmap.last_checked))                                                         \
    X(last_modified, static_cast<double>(beatmap.last_modified))                                                       \
    X(last_modification_time, static_cast<double>(beatmap.last_modification_time))                                     \
    X(mania_scroll_speed, static_cast<double>(beatmap.mania_scroll_speed))                                             \
    X(sliders, static_cast<double>(beatmap.sliders))                                                                   \
    X(spinners, static_cast<double>(beatmap.spinners))                                                                 \
    X(hitcircle, static_cast<double>(beatmap.hitcircle))

namespace osu_filter {
    enum class query_field_kind { text, number, enum_list, flag };

    enum class sort_field_key {
        unknown,
        artist,
        title,
        creator,
        difficulty,
        source,
        tags,
        folder_name,
        audio_file_name,
        osu_file_name,
        star,
        bpm,
        duration,
        ar,
        cs,
        od,
        hp,
        length,
        drain_time,
        total_time,
        audio_preview_time,
        mode,
        ranked_status,
        beatmap_id,
        difficulty_id,
        thread_id,
        last_played,
        last_checked,
        last_modified,
        last_modification_time,
        mania_scroll_speed,
        sliders,
        spinners,
        hitcircle
    };

    static constexpr query_field_kind get_query_field_kind(query_field field) {
        switch (field) {
#define FIELD_KIND_CASE(field_value_type, field_kind, field_name, ...)                                                 \
    case query_field::field_name:                                                                                      \
        return query_field_kind::field_kind;
            OSU_QUERY_FIELD_LIST(FIELD_KIND_CASE)
#undef FIELD_KIND_CASE
        }
        return query_field_kind::number;
    }

    std::string to_lower_copy(std::string_view value) {
        std::string out(value);
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    struct operator_entry {
        std::string_view text;
        query_op op;
    };

    static bool match_query_operator(std::string_view query, size_t index, query_op& out_op, size_t& out_len) {
        static const operator_entry OPS[] = {
#define OP_ENTRY(token, op_name) {token, query_op::op_name},
            OSU_QUERY_OPERATOR_LIST(OP_ENTRY)
#undef OP_ENTRY
        };

        for (const auto& entry : OPS) {
            if (index + entry.text.size() <= query.size() && query.compare(index, entry.text.size(), entry.text) == 0) {
                out_op = entry.op;
                out_len = entry.text.size();
                return true;
            }
        }
        return false;
    }

    static bool is_numeric_operator(query_op op) {
        return op == query_op::eq || op == query_op::ne || op == query_op::lt || op == query_op::lte ||
               op == query_op::gt || op == query_op::gte;
    }

    static bool is_text_operator(query_op op) {
        return op == query_op::eq || op == query_op::ne || op == query_op::contains || op == query_op::not_contains ||
               op == query_op::starts_with || op == query_op::ends_with;
    }

    static bool is_operator_supported(query_field field, query_op op) {
        const query_field_kind kind = get_query_field_kind(field);
        if (kind == query_field_kind::number) {
            return is_numeric_operator(op);
        }
        if (kind == query_field_kind::text) {
            return is_text_operator(op);
        }
        if (kind == query_field_kind::enum_list) {
            return op == query_op::eq || op == query_op::ne;
        }
        return op == query_op::eq || op == query_op::ne;
    }

    static bool parse_numeric(const std::string& value, double& out_number) {
        const char* begin = value.data();
        const char* end = value.data() + value.size();
        auto result = std::from_chars(begin, end, out_number);
        if (result.ec != std::errc() || result.ptr != end) {
            return false;
        }
        return std::isfinite(out_number);
    }

    static std::string trim_copy(std::string_view value) {
        size_t start = 0;
        size_t end = value.size();
        while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
            start++;
        }
        while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
            end--;
        }
        return std::string(value.substr(start, end - start));
    }

    static std::vector<std::string> split_csv(std::string_view value) {
        std::vector<std::string> parts;
        std::string current;
        for (char c : value) {
            if (c == ',') {
                std::string trimmed = trim_copy(current);
                if (!trimmed.empty()) {
                    parts.push_back(std::move(trimmed));
                }
                current.clear();
            } else {
                current.push_back(c);
            }
        }
        std::string trimmed = trim_copy(current);
        if (!trimmed.empty()) {
            parts.push_back(std::move(trimmed));
        }
        return parts;
    }

    static bool parse_mode_token(const std::string& value, std::vector<int32_t>& out_modes, std::string& out_err) {
        static const std::unordered_map<std::string, int32_t> MODE_MAP = {
            {"osu", 0}, {"o", 0},      {"taiko", 1}, {"t", 1},     {"catch", 2},
            {"c", 2},   {"fruits", 2}, {"f", 2},     {"mania", 3}, {"m", 3},
        };

        for (const auto& token : split_csv(to_lower_copy(value))) {
            auto it = MODE_MAP.find(token);
            if (it == MODE_MAP.end()) {
                out_err = "invalid mode value";
                return false;
            }
            out_modes.push_back(it->second);
        }
        return true;
    }

    static bool parse_status_token(const std::string& value, std::vector<int32_t>& out_status, std::string& out_err) {
        static const std::unordered_map<std::string, int32_t> STATUS_MAP = {
            {"ranked", 4},       {"r", 4}, {"approved", 5}, {"a", 5}, {"pending", 2}, {"p", 2},
            {"notsubmitted", 1}, {"n", 1}, {"unknown", 0},  {"u", 0}, {"loved", 7},   {"l", 7},
        };

        for (const auto& token : split_csv(to_lower_copy(value))) {
            auto it = STATUS_MAP.find(token);
            if (it == STATUS_MAP.end()) {
                out_err = "invalid status value";
                return false;
            }
            out_status.push_back(it->second);
        }
        return true;
    }

    static void register_aliases(std::unordered_map<std::string, query_field>& map, query_field field,
                                 std::initializer_list<std::string_view> aliases) {
        for (const std::string_view alias : aliases) {
            if (!alias.empty()) {
                map.emplace(std::string(alias), field);
            }
        }
    }

    static std::optional<query_field> parse_query_field(const std::string& raw_key) {
        const std::string key = to_lower_copy(raw_key);
        static const std::unordered_map<std::string, query_field> FIELD_BY_ALIAS = [] {
            std::unordered_map<std::string, query_field> map;
            map.reserve(32);
#define INSERT_QUERY_FIELD_ALIASES(field_value_type, field_kind, field_name, ...)                                      \
    register_aliases(map, query_field::field_name, {#field_name, ##__VA_ARGS__});
            OSU_QUERY_FIELD_LIST(INSERT_QUERY_FIELD_ALIASES)
#undef INSERT_QUERY_FIELD_ALIASES
            return map;
        }();
        auto it = FIELD_BY_ALIAS.find(key);
        if (it != FIELD_BY_ALIAS.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    static bool parse_bool_token(std::string_view value, bool& out_flag) {
        const std::string lower = to_lower_copy(trim_copy(value));
        if (lower == "1" || lower == "true" || lower == "yes") {
            out_flag = true;
            return true;
        }
        if (lower == "0" || lower == "false" || lower == "no") {
            out_flag = false;
            return true;
        }
        return false;
    }

    bool parse_query(const std::string& query, osu_db_filter_props& out, std::string& err) {
        out.has_query = true;

        auto push_text_tokens = [&](const std::string& text) {
            std::string lower = to_lower_copy(text);
            std::string token;

            for (char c : lower) {
                if (std::isspace(static_cast<unsigned char>(c))) {
                    if (!token.empty()) {
                        out.query_text_tokens.push_back(token);
                        token.clear();
                    }
                } else {
                    token.push_back(c);
                }
            }
            if (!token.empty()) {
                out.query_text_tokens.push_back(token);
            }
        };

        size_t i = 0;
        const size_t len = query.size();

        while (i < len) {
            while (i < len && std::isspace(static_cast<unsigned char>(query[i]))) {
                i++;
            }
            if (i >= len) {
                break;
            }

            const size_t token_start = i;
            while (i < len && (std::isalnum(static_cast<unsigned char>(query[i])) || query[i] == '_')) {
                i++;
            }

            if (i == token_start) {
                size_t end = i;
                while (end < len && !std::isspace(static_cast<unsigned char>(query[end]))) {
                    end++;
                }
                push_text_tokens(query.substr(i, end - i));
                i = end;
                continue;
            }

            std::string key = query.substr(token_start, i - token_start);

            query_op parsed_op = query_op::eq;
            size_t op_len = 0;
            if (!match_query_operator(query, i, parsed_op, op_len)) {
                push_text_tokens(key);
                continue;
            }
            i += op_len;

            while (i < len && std::isspace(static_cast<unsigned char>(query[i]))) {
                i++;
            }

            std::string value;
            if (i < len && query[i] == '"') {
                i++;
                size_t start = i;
                while (i < len && query[i] != '"') {
                    i++;
                }
                value = query.substr(start, i - start);
                if (i < len && query[i] == '"') {
                    i++;
                }
            } else {
                size_t start = i;
                while (i < len && !std::isspace(static_cast<unsigned char>(query[i]))) {
                    i++;
                }
                value = query.substr(start, i - start);
            }

            auto field = parse_query_field(key);
            if (!field.has_value()) {
                if (!value.empty()) {
                    push_text_tokens(value);
                }
                continue;
            }

            if (value.empty() && field.value() != query_field::unplayed) {
                continue;
            }

            query_filter filter;
            filter.field = field.value();
            filter.op = parsed_op;
            if (!is_operator_supported(filter.field, filter.op)) {
                err = "invalid operator for query field";
                return false;
            }

            const query_field_kind kind = get_query_field_kind(filter.field);

            if (kind == query_field_kind::enum_list && filter.field == query_field::mode) {
                if (!parse_mode_token(value, filter.int_list, err)) {
                    return false;
                }
            } else if (kind == query_field_kind::enum_list && filter.field == query_field::status) {
                if (!parse_status_token(value, filter.int_list, err)) {
                    return false;
                }
            } else if (kind == query_field_kind::flag) {
                if (value.empty()) {
                    filter.flag = true;
                } else if (!parse_bool_token(value, filter.flag)) {
                    err = "invalid boolean filter";
                    return false;
                }
            } else if (kind == query_field_kind::text) {
                filter.text = to_lower_copy(value);
            } else if (kind == query_field_kind::number) {
                double num = 0.0;
                if (!parse_numeric(value, num)) {
                    err = "invalid numeric filter";
                    return false;
                }
                filter.number = num;
                filter.has_number = true;
            } else {
                err = "invalid query field";
                return false;
            }

            out.query_filters.push_back(std::move(filter));
        }

        return true;
    }

    template <typename T> static bool in_list(const std::vector<T>& values, const T& target) {
        if (values.empty()) {
            return true;
        }
        return std::find(values.begin(), values.end(), target) != values.end();
    }

    static bool range_ok(const range_filter& range, double value) {
        if (range.has_min && value < range.min) {
            return false;
        }
        if (range.has_max && value > range.max) {
            return false;
        }
        return true;
    }

    static double get_nomod_star_rating(const std::vector<osu_int_float_pair>& ratings) {
        if (ratings.empty()) {
            return 0.0;
        }
        for (const auto& pair : ratings) {
            if (pair.mod_combination == 0) {
                return pair.star_rating;
            }
        }
        return ratings.front().star_rating;
    }

    static double get_star_rating_for_mode(const osu_db_beatmap& beatmap) {
        switch (beatmap.mode) {
            case 1:
                return get_nomod_star_rating(beatmap.star_rating_taiko);
            case 2:
                return get_nomod_star_rating(beatmap.star_rating_ctb);
            case 3:
                return get_nomod_star_rating(beatmap.star_rating_mania);
            case 0:
            default:
                return get_nomod_star_rating(beatmap.star_rating_standard);
        }
    }

    double get_common_bpm(const std::vector<osu_db_timing_point>& timing_points, int32_t length) {
        if (timing_points.empty()) {
            return 0.0;
        }

        const double last_time = length > 0 ? static_cast<double>(length) : timing_points.back().offset;
        std::unordered_map<int32_t, double> duration_by_bpm;

        for (size_t i = 0; i < timing_points.size(); i++) {
            const auto& point = timing_points[i];
            if (point.offset > last_time) {
                continue;
            }

            if (point.bpm == 0.0) {
                continue;
            }

            const double bpm_raw = 60000.0 / point.bpm;
            const double bpm = std::round(bpm_raw * 1000.0) / 1000.0;
            const double current_time = i == 0 ? 0.0 : timing_points[i].offset;
            const double next_time = i + 1 >= timing_points.size() ? last_time : timing_points[i + 1].offset;
            const double duration = next_time - current_time;

            int32_t bpm_key = static_cast<int32_t>(std::round(bpm * 1000.0));
            duration_by_bpm[bpm_key] += duration;
        }

        double best_duration = 0.0;
        double best_bpm = 0.0;
        for (const auto& pair : duration_by_bpm) {
            if (pair.second > best_duration) {
                best_duration = pair.second;
                best_bpm = static_cast<double>(pair.first) / 1000.0;
            }
        }

        return best_bpm;
    }

    static bool contains_case_insensitive(std::string_view haystack, std::string_view needle) {
        if (needle.empty()) {
            return true;
        }
        if (needle.size() > haystack.size()) {
            return false;
        }
        return std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), [](char left, char right) {
                   return std::tolower(static_cast<unsigned char>(left)) ==
                          std::tolower(static_cast<unsigned char>(right));
               }) != haystack.end();
    }

    static bool compare_numeric(double value, double target, query_op op) {
        switch (op) {
            case query_op::eq:
                return value == target;
            case query_op::ne:
                return value != target;
            case query_op::lt:
                return value < target;
            case query_op::lte:
                return value <= target;
            case query_op::gt:
                return value > target;
            case query_op::gte:
                return value >= target;
            case query_op::contains:
            case query_op::not_contains:
            case query_op::starts_with:
            case query_op::ends_with:
                return false;
        }
        return false;
    }

    static bool starts_with_case_insensitive(std::string_view value, std::string_view prefix) {
        if (prefix.empty()) {
            return true;
        }
        if (prefix.size() > value.size()) {
            return false;
        }
        for (size_t i = 0; i < prefix.size(); i++) {
            if (std::tolower(static_cast<unsigned char>(value[i])) !=
                std::tolower(static_cast<unsigned char>(prefix[i]))) {
                return false;
            }
        }
        return true;
    }

    static bool ends_with_case_insensitive(std::string_view value, std::string_view suffix) {
        if (suffix.empty()) {
            return true;
        }
        if (suffix.size() > value.size()) {
            return false;
        }
        const size_t offset = value.size() - suffix.size();
        for (size_t i = 0; i < suffix.size(); i++) {
            if (std::tolower(static_cast<unsigned char>(value[offset + i])) !=
                std::tolower(static_cast<unsigned char>(suffix[i]))) {
                return false;
            }
        }
        return true;
    }

    static bool compare_text(std::string_view value, std::string_view target, query_op op) {
        switch (op) {
            case query_op::eq:
            case query_op::contains:
                return contains_case_insensitive(value, target);
            case query_op::ne:
            case query_op::not_contains:
                return !contains_case_insensitive(value, target);
            case query_op::starts_with:
                return starts_with_case_insensitive(value, target);
            case query_op::ends_with:
                return ends_with_case_insensitive(value, target);
            default:
                return false;
        }
    }

    static double ticks_to_days_since(int64_t ticks) {
        if (ticks <= 0) {
            return -1.0;
        }

        constexpr int64_t unix_epoch_ticks = 621355968000000000LL;
        constexpr int64_t ticks_per_millisecond = 10000LL;
        constexpr double milliseconds_per_day = 1000.0 * 60.0 * 60.0 * 24.0;

        const int64_t unix_ticks = ticks - unix_epoch_ticks;
        const double unix_ms = static_cast<double>(unix_ticks) / static_cast<double>(ticks_per_millisecond);
        const auto now = std::chrono::system_clock::now();
        const double now_ms =
            static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
        return (now_ms - unix_ms) / milliseconds_per_day;
    }

    using text_field_fn = std::string_view (*)(const osu_db_beatmap&);
    using num_field_fn = double (*)(const osu_db_beatmap&);

    static bool matches_query_filter(const osu_db_beatmap& beatmap, const query_filter& filter) {
        static const std::unordered_map<query_field, text_field_fn> TEXT_DISPATCH = {
#define X(name, expr) {query_field::name, [](const osu_db_beatmap& beatmap) -> std::string_view { return expr; }},
            OSU_TEXT_QUERY_ACCESSORS(X)
#undef X
        };

        static const std::unordered_map<query_field, num_field_fn> NUM_DISPATCH = {
#define X(name, expr)                                                                                                  \
    {query_field::name, [](const osu_db_beatmap& beatmap) -> double { return static_cast<double>(expr); }},
            OSU_NUMBER_QUERY_ACCESSORS(X)
#undef X
        };

        auto text_it = TEXT_DISPATCH.find(filter.field);
        if (text_it != TEXT_DISPATCH.end()) {
            return compare_text(text_it->second(beatmap), filter.text, filter.op);
        }

        auto num_it = NUM_DISPATCH.find(filter.field);
        if (num_it != NUM_DISPATCH.end()) {
            return filter.has_number && compare_numeric(num_it->second(beatmap), filter.number, filter.op);
        }

        switch (filter.field) {
            case query_field::played: {
                if (!filter.has_number) {
                    return false;
                }
                const double days = ticks_to_days_since(beatmap.last_played);
                return days >= 0.0 && compare_numeric(days, filter.number, filter.op);
            }
            case query_field::mode:
            case query_field::status: {
                if (filter.int_list.empty()) {
                    return false;
                }
                const int32_t value = filter.field == query_field::mode ? beatmap.mode : beatmap.ranked_status;
                const bool matched = in_list(filter.int_list, value);
                return filter.op == query_op::ne ? !matched : matched;
            }
            case query_field::unplayed: {
                const bool is_unplayed = beatmap.unplayed != 0 || beatmap.last_played <= 0;
                return filter.op == query_op::ne ? (is_unplayed != filter.flag) : (is_unplayed == filter.flag);
            }
            default:
                return false;
        }
    }

    static bool matches_default_text_token(const osu_db_beatmap& beatmap, const std::string& token) {
        return contains_case_insensitive(beatmap.artist, token) || contains_case_insensitive(beatmap.creator, token) ||
               contains_case_insensitive(beatmap.title, token) ||
               contains_case_insensitive(beatmap.difficulty, token) ||
               contains_case_insensitive(beatmap.source, token) || contains_case_insensitive(beatmap.tags, token);
    }

    bool matches_filter(const osu_db_beatmap& beatmap, const osu_db_filter_props& props) {
        if (!in_list(props.modes, beatmap.mode) || !in_list(props.ranked_statuses, beatmap.ranked_status) ||
            !in_list(props.beatmap_ids, beatmap.beatmap_id) || !in_list(props.difficulty_ids, beatmap.difficulty_id) ||
            !in_list(props.thread_ids, beatmap.thread_id)) {
            return false;
        }

        if (!props.md5_list.empty()) {
            const std::string md5_lower = to_lower_copy(beatmap.md5);
            if (!in_list(props.md5_list, md5_lower)) {
                return false;
            }
        }

        if (props.has_artist && !contains_case_insensitive(beatmap.artist, props.artist)) {
            return false;
        }
        if (props.has_title && !contains_case_insensitive(beatmap.title, props.title)) {
            return false;
        }
        if (props.has_creator && !contains_case_insensitive(beatmap.creator, props.creator)) {
            return false;
        }
        if (props.has_difficulty && !contains_case_insensitive(beatmap.difficulty, props.difficulty)) {
            return false;
        }
        if (props.has_source && !contains_case_insensitive(beatmap.source, props.source)) {
            return false;
        }
        if (props.has_tags && !contains_case_insensitive(beatmap.tags, props.tags)) {
            return false;
        }
        if (props.has_folder_name && !contains_case_insensitive(beatmap.folder_name, props.folder_name)) {
            return false;
        }
        if (props.has_audio_file_name && !contains_case_insensitive(beatmap.audio_file_name, props.audio_file_name)) {
            return false;
        }
        if (props.has_osu_file_name && !contains_case_insensitive(beatmap.osu_file_name, props.osu_file_name)) {
            return false;
        }

        if (props.has_query) {
            for (const auto& filter : props.query_filters) {
                if (!matches_query_filter(beatmap, filter)) {
                    return false;
                }
            }

            if (!props.query_text_tokens.empty()) {
                for (const auto& token : props.query_text_tokens) {
                    if (!matches_default_text_token(beatmap, token)) {
                        return false;
                    }
                }
            }
        }

        if (props.has_ar && !range_ok(props.ar, beatmap.approach_rate)) {
            return false;
        }
        if (props.has_cs && !range_ok(props.cs, beatmap.circle_size)) {
            return false;
        }
        if (props.has_hp && !range_ok(props.hp, beatmap.hp_drain)) {
            return false;
        }
        if (props.has_od && !range_ok(props.od, beatmap.overall_difficulty)) {
            return false;
        }
        if (props.has_drain_time && !range_ok(props.drain_time, static_cast<double>(beatmap.drain_time))) {
            return false;
        }
        if (props.has_total_time && !range_ok(props.total_time, static_cast<double>(beatmap.total_time))) {
            return false;
        }
        if (props.has_duration) {
            if (!beatmap.duration.has_value()) {
                return false;
            }
            if (!range_ok(props.duration, beatmap.duration.value())) {
                return false;
            }
        }
        if (props.has_audio_preview_time &&
            !range_ok(props.audio_preview_time, static_cast<double>(beatmap.audio_preview_time))) {
            return false;
        }

        if (props.has_star_rating) {
            double sr = get_star_rating_for_mode(beatmap);
            if (!range_ok(props.star_rating, sr)) {
                return false;
            }
        }

        return true;
    }

    struct sort_key_value {
        const osu_db_beatmap* beatmap = nullptr;
        bool is_string = false;
        std::string text;
        double number = 0.0;
        bool has_number = false;
    };

    static sort_field_key parse_sort_field(std::string_view key) {
        static const std::unordered_map<std::string, sort_field_key> SORT_FIELD_BY_KEY = [] {
            std::unordered_map<std::string, sort_field_key> map;
            map.reserve(40);
            map.emplace("artist", sort_field_key::artist);
            map.emplace("title", sort_field_key::title);
            map.emplace("creator", sort_field_key::creator);
            map.emplace("difficulty", sort_field_key::difficulty);
            map.emplace("source", sort_field_key::source);
            map.emplace("tags", sort_field_key::tags);
            map.emplace("folder_name", sort_field_key::folder_name);
            map.emplace("audio_file_name", sort_field_key::audio_file_name);
            map.emplace("osu_file_name", sort_field_key::osu_file_name);
            map.emplace("star", sort_field_key::star);
            map.emplace("bpm", sort_field_key::bpm);
            map.emplace("duration", sort_field_key::duration);
            map.emplace("ar", sort_field_key::ar);
            map.emplace("cs", sort_field_key::cs);
            map.emplace("od", sort_field_key::od);
            map.emplace("hp", sort_field_key::hp);
            map.emplace("length", sort_field_key::length);
            map.emplace("drain_time", sort_field_key::drain_time);
            map.emplace("total_time", sort_field_key::total_time);
            map.emplace("audio_preview_time", sort_field_key::audio_preview_time);
            map.emplace("mode", sort_field_key::mode);
            map.emplace("ranked_status", sort_field_key::ranked_status);
            map.emplace("beatmap_id", sort_field_key::beatmap_id);
            map.emplace("difficulty_id", sort_field_key::difficulty_id);
            map.emplace("thread_id", sort_field_key::thread_id);
            map.emplace("last_played", sort_field_key::last_played);
            map.emplace("last_checked", sort_field_key::last_checked);
            map.emplace("last_modified", sort_field_key::last_modified);
            map.emplace("last_modification_time", sort_field_key::last_modification_time);
            map.emplace("mania_scroll_speed", sort_field_key::mania_scroll_speed);
            map.emplace("sliders", sort_field_key::sliders);
            map.emplace("spinners", sort_field_key::spinners);
            map.emplace("hitcircle", sort_field_key::hitcircle);
            return map;
        }();
        const std::string lower = to_lower_copy(key);
        auto it = SORT_FIELD_BY_KEY.find(lower);
        return it == SORT_FIELD_BY_KEY.end() ? sort_field_key::unknown : it->second;
    }

    using sort_text_fn = std::string (*)(const osu_db_beatmap&);
    using sort_num_fn = double (*)(const osu_db_beatmap&);

    static sort_key_value build_sort_key(const osu_db_beatmap& beatmap, sort_field_key field_key) {
        static const std::unordered_map<sort_field_key, sort_text_fn> TEXT_SORT_DISPATCH = {
#define X(name, expr) {sort_field_key::name, [](const osu_db_beatmap& beatmap) -> std::string { return expr; }},
            OSU_TEXT_SORT_ACCESSORS(X)
#undef X
        };

        static const std::unordered_map<sort_field_key, sort_num_fn> NUM_SORT_DISPATCH = {
#define X(name, expr)                                                                                                  \
    {sort_field_key::name, [](const osu_db_beatmap& beatmap) -> double { return static_cast<double>(expr); }},
            OSU_NUMBER_SORT_ACCESSORS(X)
#undef X
        };

        sort_key_value out;
        out.beatmap = &beatmap;

        auto text_it = TEXT_SORT_DISPATCH.find(field_key);
        if (text_it != TEXT_SORT_DISPATCH.end()) {
            out.is_string = true;
            out.text = text_it->second(beatmap);
            return out;
        }

        auto num_it = NUM_SORT_DISPATCH.find(field_key);
        if (num_it != NUM_SORT_DISPATCH.end()) {
            out.is_string = false;
            out.has_number = true;
            out.number = num_it->second(beatmap);
            return out;
        }

        // duration is optional so it cant go in the macro table
        if (field_key == sort_field_key::duration) {
            out.is_string = false;
            out.has_number = beatmap.duration.has_value();
            out.number = out.has_number ? beatmap.duration.value() : 0.0;
            return out;
        }

        out.has_number = false;
        return out;
    }

    static void sort_matches(std::vector<const osu_db_beatmap*>& matches, const osu_db_filter_props& props) {
        if (props.sort_key.empty() || matches.size() < 2) {
            return;
        }

        const sort_field_key field_key = parse_sort_field(props.sort_key);
        if (field_key == sort_field_key::unknown) {
            return;
        }

        std::vector<sort_key_value> keys;
        keys.reserve(matches.size());
        for (const auto* beatmap : matches) {
            keys.push_back(build_sort_key(*beatmap, field_key));
        }

        const bool desc = props.sort_desc;

        std::stable_sort(keys.begin(), keys.end(), [desc](const sort_key_value& a, const sort_key_value& b) {
            if (a.is_string || b.is_string) {
                const bool a_empty = a.text.empty();
                const bool b_empty = b.text.empty();
                if (a_empty != b_empty) {
                    return !a_empty && b_empty;
                }
                if (a.text == b.text) {
                    return false;
                }
                const bool less = a.text < b.text;
                return desc ? !less : less;
            }

            if (!a.has_number && !b.has_number) {
                return false;
            }
            if (!a.has_number) {
                return false;
            }
            if (!b.has_number) {
                return true;
            }
            if (a.number == b.number) {
                return false;
            }
            const bool less = a.number < b.number;
            return desc ? !less : less;
        });

        for (size_t i = 0; i < keys.size(); i++) {
            matches[i] = keys[i].beatmap;
        }
    }

    std::vector<const osu_db_beatmap*> filter_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                            const osu_db_filter_props& props) {
        std::vector<const osu_db_beatmap*> out;
        out.reserve(beatmaps.size());

        for (const auto& beatmap : beatmaps) {
            if (matches_filter(beatmap, props)) {
                out.push_back(&beatmap);
            }
        }

        sort_matches(out, props);
        return out;
    }

    std::vector<const osu_db_beatmap*> filter_by_properties(const osu_legacy_database& db,
                                                            const osu_db_filter_props& props) {
        return filter_by_properties(db.beatmaps, props);
    }

    std::vector<std::string> filter_md5_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                      const osu_db_filter_props& props) {
        std::vector<const osu_db_beatmap*> matches = filter_by_properties(beatmaps, props);
        std::vector<std::string> out;
        out.reserve(matches.size());

        for (const auto* beatmap : matches) {
            out.push_back(beatmap->md5);
        }

        return out;
    }

    std::vector<std::string> filter_md5_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props) {
        return filter_md5_by_properties(db.beatmaps, props);
    }

    std::vector<int32_t> filter_ids_by_properties(const std::vector<osu_db_beatmap>& beatmaps,
                                                  const osu_db_filter_props& props, id_type type) {
        std::vector<const osu_db_beatmap*> matches = filter_by_properties(beatmaps, props);
        std::vector<int32_t> out;
        out.reserve(matches.size());

        for (const auto* beatmap : matches) {
            out.push_back(type == id_type::beatmap_id ? beatmap->beatmap_id : beatmap->difficulty_id);
        }

        return out;
    }

    std::vector<int32_t> filter_ids_by_properties(const osu_legacy_database& db, const osu_db_filter_props& props,
                                                  id_type type) {
        return filter_ids_by_properties(db.beatmaps, props, type);
    }
}
