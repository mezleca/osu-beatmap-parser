import fs from "fs";
import path from "path";

import { get_property } from "../../src/index";

const TEST_LOCATION = process.platform == "linux" ?
    "/home/rel/.local/share/osu-wine/osu!/Songs/" :
    "C:\\Users\\rel\\AppData\\Local\\osu!";

const main = async () => {
    console.log("reading a shit ton of files for no reason...");

    const files = fs.readdirSync(TEST_LOCATION, { recursive: true, withFileTypes: true })
        .filter((f) => {
            return (f).isFile();
        })
        .filter((f) => {
            return (f).name.includes(".osu");
        });

    const paths = files.map((f) => {
        return path.join(f.parentPath, f.name);
    });

    if (paths.length > 0) {
        const p = paths[0];
        const data = fs.readFileSync(p);

        console.log("\n\n--- single file ---");
        console.log("file:", p);
        console.log("title:", get_property(data, "Title"));
        console.log("artist", get_property(data, "Artist"));
    }
};

main().catch(console.error);
