import fs from "fs";
import path from "path";
import processor from "../build/Release/osu-beatmap-parser.node";

const TEST_LOCATION = "/home/rel/.local/share/osu-wine/osu!/Songs/";

(() => {
    const files = fs.readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => f.isFile())
        .filter((f) => f.name.includes(".osu"));
    
    const result: string[] = [];
    const start = performance.now();

    for (const file of files) {
        const location = path.join(file.parentPath, file.name);
        result.push(processor.get_property(location, "Background"));
    }

    const end = performance.now();
    console.log(`took ${end - start}ms to finish processing ${files.length} files`);

    fs.writeFileSync("result.json", JSON.stringify(result, null, 4));
})();