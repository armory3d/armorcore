"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const path = require("path");
const GraphicsApi_1 = require("./GraphicsApi");
const ImageTool_1 = require("./ImageTool");
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
class KromExporter {
    constructor(options) {
        this.options = options;
        this.sources = [];
        this.libraries = [];
        this.addSourceDirectory(path.join(options.kha, 'Sources'));
        this.projectFiles = !options.noproject;
        this.parameters = [];
    }
    haxeOptions(name, targetOptions, defines) {
        defines.push('sys_' + this.options.target);
        defines.push('sys_g1');
        defines.push('sys_g2');
        defines.push('sys_g3');
        defines.push('sys_g4');
        defines.push('sys_a1');
        defines.push('sys_a2');
        defines.push('kha_js');
        defines.push('kha_' + this.options.target);
        defines.push('kha_' + this.options.target + '_js');
        let graphics = this.options.graphics;
        if (graphics === GraphicsApi_1.GraphicsApi.Default) {
            if (process.platform === 'win32') {
                graphics = GraphicsApi_1.GraphicsApi.Direct3D11;
            }
            else if (process.platform === 'darwin') {
                graphics = GraphicsApi_1.GraphicsApi.Metal;
            }
            else {
                graphics = GraphicsApi_1.GraphicsApi.OpenGL;
            }

        }
        defines.push('kha_' + graphics);
        defines.push('kha_g1');
        defines.push('kha_g2');
        defines.push('kha_g3');
        defines.push('kha_g4');
        defines.push('kha_a1');
        defines.push('kha_a2');
        if (this.options.debug) {
            this.parameters.push('-debug');
            defines.push('js-classic');
        }
        return {
            from: this.options.from.toString(),
            to: path.join(this.sysdir(), 'krom.js.temp'),
            realto: path.join(this.sysdir(), 'krom.js'),
            sources: this.sources,
            libraries: this.libraries,
            defines: defines,
            parameters: this.parameters,
            haxeDirectory: this.options.haxe,
            system: this.sysdir(),
            language: 'js',
            width: this.width,
            height: this.height,
            name: name,
            main: this.options.main,
        };
    }
    async export(name, targetOptions, haxeOptions) {
        fs.ensureDirSync(path.join(this.options.to, this.sysdir()));
    }
    async copySound(platform, from, to, options) {
        if (options.quality < 1) {
            fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
            let ogg = await convert(from, path.join(this.options.to, this.sysdir(), to + '.ogg'), this.options.ogg);
            return [to + '.ogg'];
        }
        else {
            fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), to + '.wav'), { overwrite: true });
            return [to + '.wav'];
        }
    }
    async copyImage(platform, from, to, options, cache) {
        // let format = await ImageTool_1.exportImage(this.options.kha, from, path.join(this.options.to, this.sysdir(), to), options, undefined, false, false, cache);
        let format = await ImageTool_1.exportImage(this.options.kha, from, path.join(this.options.to, this.sysdir(), to), options, 'lz4', false, false, cache);
        return [to + '.' + format];
    }
    async copyBlob(platform, from, to) {
        fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), to), { overwrite: true });
        return [to];
    }
    async copyVideo(platform, from, to) {
        fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
        let webm = await convert(from, path.join(this.options.to, this.sysdir(), to + '.webm'), this.options.webm);
        let files = [];
        if (webm)
            files.push(to + '.webm');
        return files;
    }

    sysdir() {
        return this.systemDirectory;
    }
    setName(name) {
        this.name = name;
        this.safename = name.replace(/ /g, '-');
    }
    setSystemDirectory(systemDirectory) {
        this.systemDirectory = systemDirectory;
    }
    addSourceDirectory(path) {
        this.sources.push(path);
    }
    addLibrary(library) {
        this.libraries.push(library);
    }
    async copyFont(platform, from, to, options) {
        return await this.copyBlob(platform, from, to + '.ttf', options);
    }
}
exports.KromExporter = KromExporter;
//# sourceMappingURL=KromExporter.js.map