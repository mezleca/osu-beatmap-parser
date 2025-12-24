const IS_BROWSER = globalThis.window != undefined;

const load_native_module = () => {
    if (IS_BROWSER) {
        const wasm_proxy: any = {
            instance: null,
            get_property: (data: Uint8Array, key: string) => {
                if (wasm_proxy.instance == null) {
                    throw Error("WASM not loaded yet");
                }

                const wasm_inst = wasm_proxy.instance;
                const buffer_ptr = wasm_inst._malloc(data.length);

                wasm_inst.HEAPU8.set(data, buffer_ptr);

                const result_val = wasm_inst.get_property(
                    buffer_ptr,
                    data.length,
                    key,
                );

                wasm_inst._free(buffer_ptr);
                return result_val;
            },
            get_properties: (data: Uint8Array, keys: string[]) => {
                if (wasm_proxy.instance == null) {
                    throw Error("WASM not loaded yet");
                }

                const wasm_inst = wasm_proxy.instance;
                const buffer_ptr = wasm_inst._malloc(data.length);

                wasm_inst.HEAPU8.set(data, buffer_ptr);

                const results_obj = wasm_inst.get_properties(
                    buffer_ptr,
                    data.length,
                    keys,
                );
                was
                m_inst._free(buffer_ptr);
                return results_obj;
            },
        };

        const create_func = (globalThis as any).create_osu_parser;

        if (create_func != undefined) {
            create_func().then((instance: any) => {
                wasm_proxy.instance = instance;
            });
        }

        return wasm_proxy;
    }

    try {
        const fs_mod = require("fs");
        const path_mod = require("path");

        const load_prebuilt = () => {
            const OS_PLATFORM = process.platform;
            const CPU_ARCH = process.arch;

            const SCAN_PATHS = [
                path_mod.join(
                    __dirname,
                    "..",
                    "..",
                    "prebuilds",
                    `${OS_PLATFORM}-${CPU_ARCH}`,
                    "osu-beatmap-parser.node",
                ),
                path_mod.join(
                    __dirname,
                    "prebuilds",
                    `${OS_PLATFORM}-${CPU_ARCH}`,
                    "osu-beatmap-parser.node",
                ),
            ];

            for (const path_str of SCAN_PATHS) {
                if (fs_mod.existsSync(path_str)) {
                    return require(path_str);
                }
            }

            return false;
        };

        const load_local = () => {
            const SCAN_PATHS = [
                path_mod.join(
                    __dirname,
                    "..",
                    "..",
                    "build",
                    "osu-beatmap-parser.node",
                ),
                path_mod.join(
                    __dirname,
                    "..",
                    "..",
                    "build",
                    "Release",
                    "osu-beatmap-parser.node",
                ),
                path_mod.join(
                    __dirname,
                    "..",
                    "build",
                    "osu-beatmap-parser.node",
                ),
                path_mod.join(__dirname, "build", "osu-beatmap-parser.node"),
            ];

            for (const path_str of SCAN_PATHS) {
                if (fs_mod.existsSync(path_str)) {
                    return require(path_str);
                }
            }

            return false;
        };

        const bin_module = load_prebuilt() || load_local();

        if (bin_module) {
            return bin_module;
        }
    } catch (e) {}

    return false;
};

export let native: any = load_native_module();

try {
    if (native == false && IS_BROWSER == false) {
        throw Error("failed to find native module...");
    }
} catch (err) {
    if (IS_BROWSER) {
        native = {};
    } else {
        throw new Error("failed to load native module...\n" + err);
    }
}
