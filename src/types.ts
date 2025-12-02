export type OsuKey =
    | "AudioFilename"
    | "AudioLeadIn"
    | "PreviewTime"
    | "Countdown"
    | "SampleSet"
    | "StackLeniency"
    | "Mode"
    | "LetterboxInBreaks"
    | "WidescreenStoryboard"
    | "Bookmarks"
    | "DistanceSpacing"
    | "BeatDivisor"
    | "GridSize"
    | "TimelineZoom"
    | "Title"
    | "TitleUnicode"
    | "Artist"
    | "ArtistUnicode"
    | "Creator"
    | "Version"
    | "Source"
    | "Tags"
    | "BeatmapID"
    | "BeatmapSetID"
    | "HPDrainRate"
    | "CircleSize"
    | "OverallDifficulty"
    | "ApproachRate"
    | "SliderMultiplier"
    | "SliderTickRate"
    | "Background"
    | "Video"
    | "Storyboard"
    | "Duration";

export interface OsuInput {
    path: string;
    id?: string;
}

export interface INativeExporter {
    get_property(location: string, key: string): string;
    get_properties(location: string, keys: string[]): Record<string, string>;
    process_beatmaps(locations: string[], keys: string[]): Promise<Record<string, string>[]>;
    get_duration(location: string): number;
    get_audio_duration(location: string): number;
}
