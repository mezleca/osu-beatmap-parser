const IS_BROWSER = globalThis.window != undefined;

interface WasmProxy {
    instance: any | null;
    init_promise: Promise<void> | null;
    get_property: (data: Uint8Array, key: string) => string;
    get_properties: (data: Uint8Array, keys: string[]) => any;
}

const wasm_proxy: WasmProxy = {
    instance: null,
    init_promise: null,
    get_property: (data: Uint8Array, key: string) => {
        if (wasm_proxy.instance == null) throw Error("WASM not loaded yet...");

        const wasm_inst = wasm_proxy.instance;
        const buffer_ptr = wasm_inst._malloc(data.length);
        wasm_inst.HEAPU8.set(data, buffer_ptr);

        const result = wasm_inst.get_property(buffer_ptr, data.length, key);

        wasm_inst._free(buffer_ptr);
        return result;
    },
    get_properties: (data: Uint8Array, keys: string[]) => {
        if (wasm_proxy.instance == null) throw Error("WASM not loaded yet...");

        const wasm_inst = wasm_proxy.instance;
        const buffer_ptr = wasm_inst._malloc(data.length);
        wasm_inst.HEAPU8.set(data, buffer_ptr);

        const result = wasm_inst.get_properties(buffer_ptr, data.length, keys);

        wasm_inst._free(buffer_ptr);
        return result;
    },
};

export const init_wasm = async () => {
    if (!IS_BROWSER) return;
    if (wasm_proxy.instance) return;
    if (wasm_proxy.init_promise) return wasm_proxy.init_promise;

    wasm_proxy.init_promise = (async () => {
        // try dynamic import first, fallback to globalThis
        let create_func;

        try {
            // @ts-ignore
            const mod = await import("../../build/osu-beatmap-parser.js");
            create_func = mod.default || mod.create_osu_parser;
        } catch (e) {
            create_func = (globalThis as any).create_osu_parser;
        }

        if (!create_func) {
            throw Error("create_osu_parser not found");
        }

        wasm_proxy.instance = await create_func();
    })();

    return wasm_proxy.init_promise;
};

const load_native_module = () => {
    if (IS_BROWSER) {
        return wasm_proxy;
    }

    try {
        const fs = require("fs");
        const path = require("path");
        const platform = process.platform;
        const arch = process.arch;

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

            // local builds
            path.join(
                __dirname,
                "..",
                "..",
                "build",
                "osu-beatmap-parser.node"
            ),
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
            if (fs.existsSync(p)) {
                return require(p);
            }
        }
    } catch (e) {}

    return false;
};

export let native: any = load_native_module();

if (native == false && !IS_BROWSER) {
    throw new Error("failed to load native module...");
}
