import { OsuKey, OsuInput } from "./types";
export declare function get_property(location: string, key: OsuKey): string;
export declare function get_properties(input: string | OsuInput, keys: OsuKey[]): Record<OsuKey, string> & {
    id?: string;
};
export declare function process_beatmaps(inputs: (string | OsuInput)[], keys: OsuKey[]): Promise<(Record<OsuKey, string> & {
    id?: string;
})[]>;
export declare function get_duration(location: string): number;
export declare function get_audio_duration(location: string): number;
export { OsuKey, OsuInput };
