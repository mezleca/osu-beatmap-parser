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
    data: Uint8Array;
    id?: string;
}

export interface INativeExporter {
    get_property(data: Uint8Array, key: string): string;
    get_properties(data: Uint8Array, keys: string[]): Record<string, string>;
}
