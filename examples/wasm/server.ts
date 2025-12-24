const SERVER_PORT = 3000;

console.log(`http://localhost:${SERVER_PORT}`);

Bun.serve({
    port: 3000,
    async fetch(req) {
        const url_obj = new URL(req.url);
        const path_str = url_obj.pathname;

        try {
            if (path_str == "/" || path_str == "/index.html") {
                return new Response(Bun.file(import.meta.dir + "/index.html"));
            }

            if (path_str == "/app.js") {
                const build_result = await Bun.build({
                    entrypoints: [import.meta.dir + "/app.ts"],
                    target: "browser",
                    minify: false,
                });

                if (!build_result.success) {
                    return new Response(build_result.logs.join("\n"), {
                        status: 500,
                    });
                }

                return new Response(build_result.outputs[0]);
            }

            const STATIC_FILE = Bun.file(import.meta.dir + path_str);

            if (await STATIC_FILE.exists()) {
                return new Response(STATIC_FILE);
            }

            if (path_str.startsWith("/build/")) {
                const BUILD_ARTIFACT = Bun.file(
                    import.meta.dir + "/../../" + path_str
                );

                if (await BUILD_ARTIFACT.exists()) {
                    return new Response(BUILD_ARTIFACT);
                }
            }

            return new Response("Not Found", { status: 404 });
        } catch (e: any) {
            return new Response(e.message, { status: 500 });
        }
    },
});
