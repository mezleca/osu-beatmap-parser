"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const fs_1 = __importDefault(require("fs"));
const path_1 = __importDefault(require("path"));
const osu_beatmap_processor_node_1 = __importDefault(require("../build/Release/osu-beatmap-parser.node"));
const TEST_LOCATION = "/home/rel/.local/share/osu-wine/osu!/Songs/";
(() => {
    const files = fs_1.default.readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => f.isFile())
        .filter((f) => f.name.includes(".osu"));
    const result = [];
    const start = performance.now();
    for (const file of files) {
        const location = path_1.default.join(file.parentPath, file.name);
        result.push(osu_beatmap_processor_node_1.default.get_property(location, "Background"));
    }
    const end = performance.now();
    console.log(`took ${end - start}ms to finish processing ${files.length} files`);
    fs_1.default.writeFileSync("result.json", JSON.stringify(result, null, 4));
})();
