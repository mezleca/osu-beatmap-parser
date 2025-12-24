import { get_property, init_wasm } from "../../src/index";

const log_status = (msg: string, color = "black") => {
    const el = document.getElementById("status");
    if (el) {
        el.innerText = msg;
        el.style.color = color;
    }
};

const parse_file = async (name: string) => {
    try {
        const res = await fetch(name);
        const buf = await res.arrayBuffer();
        const data_arr = new Uint8Array(buf);

        const title = get_property(data_arr, "Title");
        const artist = get_property(data_arr, "Artist");

        const div = document.createElement("div");
        div.className = "node";
        div.innerText = `File: ${name}\nTitle: ${title}\nArtist: ${artist}`;
        document.getElementById("results")?.appendChild(div);
    } catch (err) {
        console.error(`fail load ${name}:`, err);
    }
};

const init = async () => {
    try {
        log_status("initializing wasm...");
        await init_wasm();
        log_status("wasm loaded!", "green");

        await parse_file("1.osu");
        await parse_file("2.osu");
    } catch (e: any) {
        log_status("fail load wasm: " + e.message, "red");
        console.error(e);
    }
};

init();
