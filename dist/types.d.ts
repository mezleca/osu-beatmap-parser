export type OsuKey = "AudioFilename" | "AudioLeadIn" | "PreviewTime" | "Countdown" | "SampleSet" | "StackLeniency" | "Mode" | "LetterboxInBreaks" | "WidescreenStoryboard" | "Bookmarks" | "DistanceSpacing" | "BeatDivisor" | "GridSize" | "TimelineZoom" | "Title" | "TitleUnicode" | "Artist" | "ArtistUnicode" | "Creator" | "Version" | "Source" | "Tags" | "BeatmapID" | "BeatmapSetID" | "HPDrainRate" | "CircleSize" | "OverallDifficulty" | "ApproachRate" | "SliderMultiplier" | "SliderTickRate" | "Background" | "Video" | "Storyboard" | "Duration";
export interface OsuInput {
    path: string;
    id?: string;
}
