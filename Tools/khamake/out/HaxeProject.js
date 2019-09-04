"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs-extra");
const path = require("path");
function hxml(projectdir, options) {
    let data = '';
    let lines = [];
    // returns only unique lines and '' otherwise
    function unique(line) {
        if (lines.indexOf(line) === -1) {
            lines.push(line);
            return line;
        }
        return '';
    }
    for (let i = 0; i < options.sources.length; ++i) {
        if (path.isAbsolute(options.sources[i])) {
            data += unique('-cp ' + options.sources[i] + '\n');
        }
        else {
            data += unique('-cp ' + path.relative(projectdir, path.resolve(options.from, options.sources[i])) + '\n'); // from.resolve('build').relativize(from.resolve(this.sources[i])).toString());
        }
    }
    for (let i = 0; i < options.libraries.length; ++i) {
        if (path.isAbsolute(options.libraries[i].libpath)) {
            data += unique('-cp ' + options.libraries[i].libpath + '\n');
        }
        else {
            data += unique('-cp ' + path.relative(projectdir, path.resolve(options.from, options.libraries[i].libpath)) + '\n'); // from.resolve('build').relativize(from.resolve(this.sources[i])).toString());
        }
    }
    for (let d in options.defines) {
        let define = options.defines[d];
        data += unique('-D ' + define + '\n');
    }
    if (options.language === 'js') {
        data += unique('-js ' + path.normalize(options.to) + '\n');
    }
    for (let param of options.parameters) {
        data += unique(param + '\n');
    }
    if (!options.parameters.some((param) => param.includes('-main '))) {
        const entrypoint = options ? options.main ? options.main : 'Main' : 'Main';
        data += unique('-main ' + entrypoint + '\n');
    }
    fs.outputFileSync(path.join(projectdir, 'project-' + options.system + '.hxml'), data);
}
function writeHaxeProject(projectdir, projectFiles, options) {
    hxml(projectdir, options);
}
exports.writeHaxeProject = writeHaxeProject;
//# sourceMappingURL=HaxeProject.js.map