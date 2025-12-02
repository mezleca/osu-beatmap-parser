import bindings from "bindings";
import { OsuKey, OsuInput, INativeExporter } from "./types";

const native = bindings("osu-beatmap-parser") as INativeExporter;


export function get_property(location: string, key: OsuKey): string {
    return native.get_property(location, key);
};

export function get_properties(input: string | OsuInput, keys: OsuKey[]): Record<OsuKey, string> & { id?: string } {
    const location = typeof input === "string" ? input : input.path;
    const result = native.get_properties(location, keys) as Record<OsuKey, string>;

    if (typeof input !== "string" && input.id) {
        return { ...result, id: input.id };
    }

    return result;
};

export async function process_beatmaps(inputs: (string | OsuInput)[], keys: OsuKey[]): Promise<(Record<OsuKey, string> & { id?: string })[]> {
    const locations = inputs.map(i => typeof i === "string" ? i : i.path);
    const results = await native.process_beatmaps(locations, keys) as Record<OsuKey, string>[];

    return results.map((res, i) => {
        const input = inputs[i];
        if (typeof input !== "string" && input.id) {
            return { ...res, id: input.id };
        }
        return res;
    });
};

export function get_duration(location: string): number {
    return native.get_duration(location);
};

export function get_audio_duration(location: string): number {
    return native.get_audio_duration(location);
};

export { OsuKey, OsuInput };