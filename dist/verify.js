"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const index_1 = require("./index");
const TEST_LOCATION = process.platform == "linux" ?
    "/home/rel/.local/share/osu-wine/osu!/Songs/" :
    "C:\\Users\\rel\\AppData\\Local\\osu!";
async function main() {
    const files = fs_1.default.readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => (f).isFile())
        .filter((f) => (f).name.includes(".osu"));
    const paths = files.map((f) => path_1.default.join(f.parentPath, f.name));
    if (paths.length > 0) {
        const p = paths[0];
        console.log("\n\n--- single file ---");
        console.log("file:", p);
        console.log("title:", (0, index_1.get_property)(p, "Title"));
        console.log("duration (audio):", (0, index_1.get_duration)(p));
    }
    const start = performance.now();
    const inputs = paths.map((p, i) => ({ path: path_1.default.resolve(p), id: `md5-${i}` }));
    const results = await (0, index_1.process_beatmaps)(inputs, ["Title", "Artist", "Creator", "Duration"]);
    const end = performance.now();
    console.log(`processed ${results.length} files in ${end - start}ms!`);
    fs_1.default.writeFileSync("./result.json", JSON.stringify(results, null, 4));
}
main().catch(console.error);
