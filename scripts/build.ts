import { spawnSync } from "child_process";

import fs from "fs";
import path from "path";

const TARGET_DIR = "build";

const execute_raw = (bin_name: string, arg_list: string[]) => {
    const full_cmd = `${bin_name} ${arg_list.join(" ")}`;
    console.log(`\nexecuting: ${full_cmd}`);

    const proc_res = spawnSync(full_cmd, {
        stdio: "inherit",
        shell: true,
    });

    if (proc_res.status != 0) {
        process.exit(proc_res.status || 1);
    }
};

const remove_dir = (dir_path: string) => {
    if (fs.existsSync(dir_path)) {
        try {
            fs.rmSync(dir_path, { recursive: true, force: true });
        } catch (e: any) {
            console.warn(`[warn] could not remove ${dir_path}: ${e.message}`);
        }
    }
};

const compile_native = () => {
    const TMP_NATIVE = path.join(TARGET_DIR, "native-tmp");

    remove_dir(TMP_NATIVE);
    execute_raw("cmake-js", ["build", "-G", "Ninja", "--out", TMP_NATIVE]);

    const BIN_NAMES = [
        path.join(TMP_NATIVE, "osu-beatmap-parser.node"),
        path.join(TMP_NATIVE, "Release", "osu-beatmap-parser.node"),
    ];

    for (const bin_file of BIN_NAMES) {
        if (fs.existsSync(bin_file)) {
            // copy to build/
            fs.copyFileSync(
                bin_file,
                path.join(TARGET_DIR, "osu-beatmap-parser.node")
            );

            // copy to prebuilds for node-gyp-build
            const platform = process.platform;
            const arch = process.arch;
            const prebuilds_dir = path.join("prebuilds", `${platform}-${arch}`);

            if (!fs.existsSync(prebuilds_dir)) {
                fs.mkdirSync(prebuilds_dir, { recursive: true });
            }

            fs.copyFileSync(
                bin_file,
                path.join(prebuilds_dir, "osu-beatmap-parser.node")
            );

            console.log(`\ncopied binary to ${prebuilds_dir}`);
            return;
        }
    }

    process.exit(1);
};

const compile_wasm = () => {
    const TMP_WASM = path.join(TARGET_DIR, "wasm-tmp");

    remove_dir(TMP_WASM);

    execute_raw("emcmake", ["cmake", "-S", ".", "-B", TMP_WASM, "-G", "Ninja"]);
    execute_raw("cmake", ["--build", TMP_WASM]);

    const emscripten_js = path.join(TMP_WASM, "osu-beatmap-parser.js");

    if (!fs.existsSync(emscripten_js)) {
        console.error("emscripten build failed - no output file");
        process.exit(1);
    }

    fs.copyFileSync(
        emscripten_js,
        path.join(TARGET_DIR, "osu-beatmap-parser.js")
    );

    // bundle wrapper with bun
    console.log("\nbundling wasm wrapper...");

    execute_raw("bun", [
        "build",
        "src/lib/wasm-wrapper.ts",
        "--outdir",
        TARGET_DIR,
        "--target",
        "browser",
        "--format",
        "iife",
        "--minify",
    ]);

    const wrapper_bundle = path.join(TARGET_DIR, "wasm-wrapper.js");

    if (!fs.existsSync(wrapper_bundle)) {
        console.error("wrapper bundle failed");
        process.exit(1);
    }

    // load content
    let emscripten_code = fs.readFileSync(emscripten_js, "utf-8");
    const wrapper_code = fs.readFileSync(wrapper_bundle, "utf-8");

    const final_bundle = `${emscripten_code};${wrapper_code};`;

    fs.writeFileSync(
        path.join(TARGET_DIR, "osu-parser.browser.js"),
        final_bundle
    );

    console.log("\nwasm bundle created: build/osu-parser.browser.js");
};

const EXEC_ARGS = process.argv.slice(2);
const BUILD_GOAL = EXEC_ARGS[0] || "native";

if (fs.existsSync(TARGET_DIR) == false) {
    fs.mkdirSync(TARGET_DIR);
}

if (BUILD_GOAL == "native") {
    compile_native();
} else if (BUILD_GOAL == "wasm") {
    compile_wasm();
} else if (BUILD_GOAL == "all") {
    compile_native();
    compile_wasm();
} else if (BUILD_GOAL == "clean") {
    remove_dir(TARGET_DIR);
} else {
    process.exit(1);
}
