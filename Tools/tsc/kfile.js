
const fs = require('fs');
globalThis.require = require;

globalThis.module = {};
globalThis.__filename = __dirname + "/tsc.js";
process.argv.push("tsc.js");
process.argv.push("main.ts");
(1, eval)(fs.readFileSync("./tsc.js") + "");

// let project = new Project("tsc");
// resolve(project);
