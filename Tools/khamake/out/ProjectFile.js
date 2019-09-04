"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const path = require("path");
const Project_1 = require("./Project");
exports.Callbacks = {
    preAssetConversion: [() => { }],
    preShaderCompilation: [() => { }],
    preHaxeCompilation: [() => { }],
    postHaxeCompilation: [() => { }]
};
async function loadProject(from, projectfile, platform) {
    return new Promise((resolve, reject) => {
        fs.readFile(path.join(from, projectfile), 'utf8', (err, data) => {
            if (err) {
                throw new Error('Error reading ' + projectfile + ' from ' + from + '.');
            }
            let resolved = false;
            let callbacks = {
                preAssetConversion: () => { },
                preShaderCompilation: () => { },
                preHaxeCompilation: () => { },
                postHaxeCompilation: () => { }
            };
            let resolver = (project) => {
                resolved = true;
                exports.Callbacks.preAssetConversion.push(callbacks.preAssetConversion);
                exports.Callbacks.preShaderCompilation.push(callbacks.preShaderCompilation);
                exports.Callbacks.preHaxeCompilation.push(callbacks.preHaxeCompilation);
                exports.Callbacks.postHaxeCompilation.push(callbacks.postHaxeCompilation);
                resolve(project);
            };
            process.on('exit', (code) => {
                if (!resolved) {
                    console.error('Error: khafile.js did not call resolve, no project created.');
                }
            });
            Project_1.Project.platform = platform;
            Project_1.Project.scriptdir = from;
            try {
                let AsyncFunction = Object.getPrototypeOf(async () => { }).constructor;
                new AsyncFunction('Project', 'platform', 'require', 'process', 'resolve', 'reject', 'callbacks', data)(Project_1.Project, platform, require, process, resolver, reject, callbacks);
            }
            catch (error) {
                reject(error);
            }
        });
    });
}
exports.loadProject = loadProject;
//# sourceMappingURL=ProjectFile.js.map