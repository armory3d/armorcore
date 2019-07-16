"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs");
const path = require("path");
const chokidar = require("chokidar");
const log = require("./log");
const exec_1 = require("./exec");
class HaxeCompiler {
    constructor(from, temp, to, resourceDir, haxeDirectory, hxml, sourceDirectories, sysdir) {
        this.ready = true;
        this.todo = false;
        this.port = '7000';
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
    close() {
        if (this.watcher)
            this.watcher.close();
        if (this.compilationServer)
            this.compilationServer.kill();
    }
    async run(watch) {
        if (watch) {
            this.watcher = chokidar.watch(this.sourceMatchers, { ignored: /[\/\\]\.git/, persistent: true, ignoreInitial: true });
            this.watcher.on('add', (file) => {
                this.scheduleCompile();
            });
            this.watcher.on('change', (file) => {
                this.scheduleCompile();
            });
            this.watcher.on('unlink', (file) => {
                this.scheduleCompile();
            });
            this.startCompilationServer();
            this.triggerCompilationServer();
        }
        else {
            try {
                await this.compile();
            }
            catch (error) {
                return Promise.reject(error);
            }
        }
        return Promise.resolve();
    }
    scheduleCompile() {
        if (this.ready) {
            this.triggerCompilationServer();
        }
        else {
            this.todo = true;
        }
    }
    runHaxeAgain(parameters, onClose) {
        let exe = 'haxe';
        let env = process.env;
        if (fs.existsSync(this.haxeDirectory) && fs.statSync(this.haxeDirectory).isDirectory()) {
            let localexe = path.resolve(this.haxeDirectory, 'haxe' + exec_1.sys());
            if (!fs.existsSync(localexe))
                localexe = path.resolve(this.haxeDirectory, 'haxe');
            if (fs.existsSync(localexe))
                exe = localexe;
            const stddir = path.resolve(this.haxeDirectory, 'std');
            if (fs.existsSync(stddir) && fs.statSync(stddir).isDirectory()) {
                env.HAXE_STD_PATH = stddir;
            }
        }
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
    runHaxeAgainAndWait(parameters) {
        return new Promise((resolve, reject) => {
            this.runHaxeAgain(parameters, (code, signal) => {
                if (code === 0) {
                    resolve();
                }
                else {
                    reject();
                }
            });
        });
    }
    static cleanHxml(hxml) {
        let params = [];
        let ignoreNext = false;
        let parameters = hxml.split('\n');
        for (let parameter of parameters) {
            if (!parameter.startsWith('-main') && !parameter.startsWith('-js')) {
                params.push(parameter);
            }
        }
        return params.join('\n');
    }
    runHaxe(parameters, onClose) {
        if (fs.existsSync(path.join(this.resourceDir, 'workers.txt'))) {
            fs.unlinkSync(path.join(this.resourceDir, 'workers.txt'));
        }
        let haxe = this.runHaxeAgain(parameters, async (code, signal) => {
            if (fs.existsSync(path.join(this.resourceDir, 'workers.txt'))) {
                let hxml = fs.readFileSync(path.join(this.from, parameters[0]), { encoding: 'utf8' });
                let workers = fs.readFileSync(path.join(this.resourceDir, 'workers.txt'), { encoding: 'utf8' });
                let lines = workers.split('\n');
                for (let line of lines) {
                    if (line.trim() === '')
                        continue;
                    log.info('Creating ' + line + ' worker.');
                    let newhxml = HaxeCompiler.cleanHxml(hxml);
                    newhxml += '-main ' + line.trim() + '\n';
                    newhxml += '-js ' + path.join(this.sysdir, line.trim()) + '.js\n';
                    newhxml += '-D kha_in_worker\n';
                    fs.writeFileSync(path.join(this.from, 'temp.hxml'), newhxml, { encoding: 'utf8' });
                    await this.runHaxeAgainAndWait(['temp.hxml']);
                }
                onClose(code, signal);
            }
            else {
                onClose(code, signal);
            }
        });
        return haxe;
    }
    startCompilationServer() {
        this.compilationServer = this.runHaxe(['--wait', this.port], (code) => {
            log.info('Haxe compilation server stopped.');
        });
    }
    triggerCompilationServer() {
        process.stdout.write('\x1Bc');
        log.info('Haxe compilation...');
        this.ready = false;
        this.todo = false;
        return new Promise((resolve, reject) => {
            this.runHaxe(['--connect', this.port, this.hxml], (code) => {
                if (this.to && fs.existsSync(path.join(this.from, this.temp))) {
                    fs.renameSync(path.join(this.from, this.temp), path.join(this.from, this.to));
                }
                this.ready = true;
                if (code === 0) {
                    process.stdout.write('\x1Bc');
                    log.info('Haxe compile end.');
                }
                else {
                    log.info('Haxe compile error.');
                }
                if (code === 0)
                    resolve();
                // (node:3630) [DEP0018] DeprecationWarning: Unhandled promise rejections are deprecated. In the future,
                // promise rejections that are not handled will terminate the Node.js process with a non-zero exit code.
                // else reject('Haxe compiler error.');
                if (this.todo) {
                    this.scheduleCompile();
                }
            });
        });
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
    static spinRename(from, to) {
        for (;;) {
            if (fs.existsSync(from)) {
                fs.renameSync(from, to);
                return;
            }
        }
    }
}
exports.HaxeCompiler = HaxeCompiler;
//# sourceMappingURL=HaxeCompiler.js.map