"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
exports.get_property = get_property;
exports.get_properties = get_properties;
exports.process_beatmaps = process_beatmaps;
exports.get_duration = get_duration;
exports.get_audio_duration = get_audio_duration;
const osu_beatmap_parser_node_1 = __importDefault(require("../build/Release/osu-beatmap-parser.node"));
function get_property(location, key) {
    return osu_beatmap_parser_node_1.default.get_property(location, key);
}
;
function get_properties(input, keys) {
    const location = typeof input === "string" ? input : input.path;
    const result = osu_beatmap_parser_node_1.default.get_properties(location, keys);
    if (typeof input !== "string" && input.id) {
        return { ...result, id: input.id };
    }
    return result;
}
;
async function process_beatmaps(inputs, keys) {
    const locations = inputs.map(i => typeof i === "string" ? i : i.path);
    const results = await osu_beatmap_parser_node_1.default.process_beatmaps(locations, keys);
    return results.map((res, i) => {
        const input = inputs[i];
        if (typeof input !== "string" && input.id) {
            return { ...res, id: input.id };
        }
        return res;
    });
}
;
function get_duration(location) {
    return osu_beatmap_parser_node_1.default.get_duration(location);
}
;
function get_audio_duration(location) {
    return osu_beatmap_parser_node_1.default.get_audio_duration(location);
}
;
