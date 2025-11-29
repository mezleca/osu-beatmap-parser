import { $ } from "bun";

// TODO: auto download cmake on windows (maybe)

(async () => {
    try {
        await $`git submodule update --recursive`;
        await $`npm run build:native && npm run build:tsc`;
    } catch (err) {
        console.error("failed to setup:", err);
    }
})();
