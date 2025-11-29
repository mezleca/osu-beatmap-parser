const { spawn } = require("child_process");

// TODO: download cmake binary on windows and add to path

const execute_cmd = (command, args = []) => {
    return new Promise((resolve, reject) => {
        const child = spawn(command, args, { stdio: "inherit" });

        child.on("close", (code) => {
            if (code == 0) resolve();
            else reject(`exit code ${code}`);
        });

        child.on("error", (err) => reject(err));
    });
};

(async () => {
    try {
        await execute_cmd("ls");
        await execute_cmd("git", ["submodule", "update", "--recursive"]);
        await execute_cmd("npm", ["run", "build:native"]);
        await execute_cmd("npm", ["run", "build:tsc"]);
    } catch (err) {
        console.error("failed to setup:", err);
    }
})();
