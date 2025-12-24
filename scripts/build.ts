import { spawnSync } from "child_process";

import fs from "fs";
import path from "path";

const TARGET_DIR = "build";

const execute_raw = (bin_name: string, arg_list: string[]) => {
    console.log(`\nexecuting: ${bin_name} ${arg_list.join(" ")}`);
    const proc_res = spawnSync(bin_name, arg_list, {
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
            fs.copyFileSync(
                bin_file,
                path.join(TARGET_DIR, "osu-beatmap-parser.node")
            );
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

    const BUNDLE_FILES = ["osu-beatmap-parser.js"];

    for (const name_str of BUNDLE_FILES) {
        const full_source = path.join(TMP_WASM, name_str);
        if (fs.existsSync(full_source)) {
            fs.copyFileSync(full_source, path.join(TARGET_DIR, name_str));
        }
    }
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
