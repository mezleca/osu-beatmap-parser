const SERVER_PORT = 3000;

console.log(`Starting WASM example server on http://localhost:${SERVER_PORT}`);

const web_server = Bun.serve({
    port: SERVER_PORT,
    async fetch(req) {
        const REQUEST_URL = new URL(req.url);
        let path_str = REQUEST_URL.pathname;

        if (path_str == "/") {
            path_str = "/index.html";
        }

        try {
            const STATIC_FILE = Bun.file(import.meta.dir + path_str);
            if (await STATIC_FILE.exists()) {
                return new Response(STATIC_FILE);
            }

            if (path_str.startsWith("/build/")) {
                const BUILD_ARTIFACT = Bun.file(import.meta.dir + "/../../" + path_str);
                if (await BUILD_ARTIFACT.exists()) {
                    return new Response(BUILD_ARTIFACT);
                }
            }

            return new Response("Not Found", { status: 404 });
        } catch (e) {
            return new Response("Internal Server Error", { status: 500 });
        }
    },
});
