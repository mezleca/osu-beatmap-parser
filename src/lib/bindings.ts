const DEBUG =
    typeof process != "undefined" && process.env?.OSU_PARSER_DEBUG == "1";

const log_debug = (...args: any[]) => {
    if (DEBUG) {
        console.log("[osu-beatmap-parser]", ...args);
    }
};

const load_native_module = () => {
    const fs = require("fs");
    const path = require("path");
    const platform = process.platform;
    const arch = process.arch;

    log_debug(`loading native module for ${platform}-${arch}`);
    log_debug(`__dirname: ${__dirname}`);

    const paths = [
        // prebuilt binaries
        path.join(
            __dirname,
            "..",
            "..",
            "prebuilds",
            `${platform}-${arch}`,
            "osu-beatmap-parser.node"
        ),
        path.join(
            __dirname,
            "..",
            "prebuilds",
            `${platform}-${arch}`,
            "osu-beatmap-parser.node"
        ),

        // local dev builds
        path.join(__dirname, "..", "..", "build", "osu-beatmap-parser.node"),
        path.join(
            __dirname,
            "..",
            "..",
            "build",
            "Release",
            "osu-beatmap-parser.node"
        ),
        path.join(__dirname, "..", "build", "osu-beatmap-parser.node"),
        path.join(__dirname, "build", "osu-beatmap-parser.node"),
    ];

    for (const p of paths) {
        const resolved = path.resolve(p);
        const exists = fs.existsSync(resolved);
        log_debug(`checking: ${resolved} -> ${exists ? "found" : "not found"}`);

        if (exists) {
            log_debug(`loading native module from: ${resolved}`);
            return require(resolved);
        }
    }

    log_debug("no native module found in any path");
    return null;
};

export const native = load_native_module();

if (native == null) {
    const platform = process.platform;
    const arch = process.arch;
    throw new Error(
        `failed to load native module for ${platform}-${arch}. ` +
            `Set OSU_PARSER_DEBUG=1 for more info.`
    );
}
