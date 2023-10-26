
const asc = false; // assemblyscript
const tsc = true; // typescript

const fs = require('fs');
globalThis.require = require;

if (asc) {
	process.argv.push("asc.js");
	process.argv.push("main.ts");
	process.argv.push("-o");
	process.argv.push("main.wasm");
	(1, eval)(fs.readFileSync("./asc.js") + "");
}

if (tsc) {
	globalThis.module = {};
	globalThis.__filename = __dirname + "/tsc.js";
	process.argv.push("tsc.js");
	process.argv.push("main.ts");
	(1, eval)(fs.readFileSync("./tsc.js") + "");
}

// let project = new Project("tsc");
// resolve(project);
