"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs");
function convert(inFilename, outFilename, encoder, args = null) {
    return new Promise((resolve, reject) => {
        if (fs.existsSync(outFilename.toString()) && fs.statSync(outFilename.toString()).mtime.getTime() > fs.statSync(inFilename.toString()).mtime.getTime()) {
            resolve(true);
            return;
        }
        if (!encoder) {
            resolve(false);
            return;
        }
        let dirend = Math.max(encoder.lastIndexOf('/'), encoder.lastIndexOf('\\'));
        let firstspace = encoder.indexOf(' ', dirend);
        let exe = encoder.substr(0, firstspace);
        let parts = encoder.substr(firstspace + 1).split(' ');
        let options = [];
        for (let i = 0; i < parts.length; ++i) {
            let foundarg = false;
            if (args !== null) {
                for (let arg in args) {
                    if (parts[i] === '{' + arg + '}') {
                        options.push(args[arg]);
                        foundarg = true;
                        break;
                    }
                }
            }
            if (foundarg)
                continue;
            if (parts[i] === '{in}')
                options.push(inFilename.toString());
            else if (parts[i] === '{out}')
                options.push(outFilename.toString());
            else
                options.push(parts[i]);
        }
        // About stdio ignore: https://stackoverflow.com/a/20792428
        let process = child_process.spawn(exe, options, { stdio: 'ignore' });
        process.on('close', (code) => {
            resolve(code === 0);
        });
    });
}
exports.convert = convert;
//# sourceMappingURL=Converter.js.map