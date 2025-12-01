import { $ } from "bun";

(async () => {
    try {
        await $`npm run build:native && npm run build:tsc`;
    } catch (err) {
        console.error("failed to setup:", err);
    }
})();
