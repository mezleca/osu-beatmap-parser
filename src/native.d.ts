declare module "*osu-beatmap-parser.node" {
    export function get_property(localtion: string, key: string): string;
    export function test_promise(): Promise<boolean>;
}