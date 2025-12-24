import { OsuKey, OsuInput } from "./types";
import { native, init_wasm } from "./lib/bindings";

export const get_property = (data: Uint8Array, key: OsuKey) => {
    return native.get_property(data, key);
};

export const get_properties = (
    input: Uint8Array | OsuInput,
    keys: OsuKey[]
) => {
    const data = input instanceof Uint8Array ? input : input.data;
    const result = native.get_properties(data, keys);

    if (!(input instanceof Uint8Array) && input.id) {
        return { ...result, id: input.id };
    }

    return result;
};

export { OsuKey, OsuInput, init_wasm };
