import nodeGypBuild from "node-gyp-build";
import path from "path";

import { INativeExporter } from "../types/types";

const native = nodeGypBuild(
    path.join(__dirname, "..", "..")
) as INativeExporter;

export { native };
