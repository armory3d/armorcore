"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs");
const os = require("os");
const path = require("path");
const chokidar = require("chokidar");
const log = require("./log");
function exec_sys() {
    if (os.platform() === 'linux') {
        return '-linux64';
    }
    else if (os.platform() === 'win32') {
        return '.exe';
    }
    else {
        return '-osx';
    }
}
class HaxeCompiler {
    constructor(from, temp, to, resourceDir, haxeDirectory, hxml, sourceDirectories, sysdir) {
        this.ready = true;
        this.from = from;
        this.temp = temp;
        this.to = to;
        this.resourceDir = resourceDir;
        this.haxeDirectory = haxeDirectory;
        this.hxml = hxml;
        this.sysdir = sysdir;
        this.sourceMatchers = [];
        for (let dir of sourceDirectories) {
            this.sourceMatchers.push(path.join(dir, '**').replace(/\\/g, '/'));
        }
    }
    close() {}
    async run(watch) {
        try {
            await this.compile();
        }
        catch (error) {
            return Promise.reject(error);
        }
        return Promise.resolve();
    }
    runHaxeAgain(parameters, onClose) {
        let exe = path.resolve(this.haxeDirectory, 'haxe' + exec_sys());
        let env = process.env;
        const stddir = path.resolve(this.haxeDirectory, 'std');
        env.HAXE_STD_PATH = stddir;
        let haxe = child_process.spawn(exe, parameters, { env: env, cwd: path.normalize(this.from) });
        haxe.stdout.on('data', (data) => {
            log.info(data.toString());
        });
        haxe.stderr.on('data', (data) => {
            log.error(data.toString());
        });
        haxe.on('close', onClose);
        return haxe;
    }
    runHaxe(parameters, onClose) {
        let haxe = this.runHaxeAgain(parameters, async (code, signal) => {
            onClose(code, signal);
        });
        return haxe;
    }
    
    compile() {
        return new Promise((resolve, reject) => {
            this.runHaxe([this.hxml], (code) => {
                if (code === 0) {
                    if (this.to && fs.existsSync(path.join(this.from, this.temp))) {
                        fs.renameSync(path.join(this.from, this.temp), path.join(this.from, this.to));
                    }
                    resolve();
                }
                else {
                    process.exitCode = 1;
                    log.error('Haxe compiler error.');
                    reject();
                }
            });
        });
    }
}
exports.HaxeCompiler = HaxeCompiler;
//# sourceMappingURL=HaxeCompiler.js.map