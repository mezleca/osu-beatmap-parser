declare module "*osu-beatmap-parser.node" {
    export function get_property(location: string, key: string): string;
    export function get_properties(location: string, keys: string[]): Record<string, string>;
    export function process_beatmaps(locations: string[], keys: string[]): Promise<Record<string, string>[]>;
    export function get_duration(location: string): number;
    export function get_audio_duration(location: string): number;
    export function test_promise(): Promise<boolean>;
}