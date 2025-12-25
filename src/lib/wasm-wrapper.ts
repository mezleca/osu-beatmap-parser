console.log("[parser] wrapper loaded");

let wasm_instance: any = null;
let init_promise: Promise<void> | null = null;

const get_factory = () => {
    return (globalThis as any).create_osu_parser;
};

export const init_wasm = async (): Promise<void> => {
    if (wasm_instance) return;
    if (init_promise) return init_promise;

    init_promise = (async () => {
        const factory = get_factory();
        if (typeof factory !== "function") {
            throw new Error("create_osu_parser not found in global scope");
        }

        try {
            wasm_instance = await factory();
        } catch (e) {
            console.error(e);
            throw e;
        }
    })();

    return init_promise;
};

export const get_property = (data: Uint8Array, key: string): string => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.get_property(buffer_ptr, data.length, key);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

export const get_properties = (
    data: Uint8Array,
    keys: string[]
): Record<string, string> => {
    if (wasm_instance == null) throw new Error("wasm not initialized");

    const buffer_ptr = wasm_instance._malloc(data.length);
    wasm_instance.HEAPU8.set(data, buffer_ptr);

    try {
        return wasm_instance.get_properties(buffer_ptr, data.length, keys);
    } finally {
        wasm_instance._free(buffer_ptr);
    }
};

if (typeof window != "undefined") {
    (window as any).beatmap_parser = {
        init_wasm,
        get_property,
        get_properties,
    };
    console.log((window as any).beatmap_parser);
    console.log("[parser] attached to window");
}
