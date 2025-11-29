import fs from "fs";
import path from "path";
import { process_beatmaps, get_property, get_duration } from "./index";

const TEST_LOCATION = "/home/rel/.local/share/osu-wine/osu!/Songs/";

async function main() {
    const files = fs.readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => (f).isFile())
        .filter((f) => (f).name.includes(".osu"));

    const paths = files.map((f) => path.join(f.parentPath, f.name));

    if (paths.length > 0) {
        const p = paths[0];
        console.log("\n\n--- single file ---");
        console.log("file:", p);
        console.log("title:", get_property(p, "Title"));
        console.log("duration (audio):", get_duration(p));
    }

    const start = performance.now();
    const inputs = paths.map((p, i) => ({ path: p, id: `md5-${i}` }));
    const results = await process_beatmaps(inputs, ["Title", "Artist", "Creator", "Duration"]);
    const end = performance.now();

    console.log(`processed ${results.length} files in ${end - start}ms!`);

    fs.writeFileSync("./result.json", JSON.stringify(results, null, 4));
}

main().catch(console.error);