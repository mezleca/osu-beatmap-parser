import { OsuKey, OsuInput } from "./types";
import { native } from "./lib/bindings";

export const get_property = (data: Uint8Array, key: OsuKey): string => {
    return native.get_property(data, key);
};

export const get_properties = (
    input: Uint8Array | OsuInput,
    keys: OsuKey[]
): Record<string, string> => {
    const data = input instanceof Uint8Array ? input : input.data;
    const result = native.get_properties(data, keys);

    if (!(input instanceof Uint8Array) && input.id) {
        return { ...result, id: input.id };
    }

    return result;
};

export { OsuKey, OsuInput };
