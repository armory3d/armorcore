"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const os = require("os");
const path = require("path");
const chokidar = require("chokidar");
const Throttle = require("promise-parallel-throttle");
const GraphicsApi_1 = require("./GraphicsApi");
const AssetConverter_1 = require("./AssetConverter");
const log = require("./log");
class CompiledShader {
    constructor() {
        this.files = [];
        this.noembed = false;
    }
}
exports.CompiledShader = CompiledShader;
class ShaderCompiler {
    constructor(exporter, platform, compiler, to, temp, builddir, options, shaderMatchers) {
        this.exporter = exporter;
        this.platform = platform;
        this.compiler = compiler;
        this.type = ShaderCompiler.findType(platform, options);
        this.options = options;
        this.to = to;
        this.temp = temp;
        this.builddir = builddir;
        this.shaderMatchers = shaderMatchers;
    }
    close() {
        if (this.watcher)
            this.watcher.close();
    }
    static findType(platform, options) {
        if (options.graphics === GraphicsApi_1.GraphicsApi.Default) {
            if (process.platform === 'win32') {
                return 'd3d11';
            }
            else if (process.platform === 'darwin') {
                return 'metal';
            }
            else {
                return options.shaderversion == 300 ? 'essl' : 'glsl'; // TODO: pass gles flag
            }
        }
        else if (options.graphics === GraphicsApi_1.GraphicsApi.Vulkan) {
            return 'spirv';
        }
        else if (options.graphics === GraphicsApi_1.GraphicsApi.Metal) {
            return 'metal';
        }
        else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL) {
            return options.shaderversion == 300 ? 'essl' : 'glsl'; // TODO: pass gles flag
        }
        else if (options.graphics === GraphicsApi_1.GraphicsApi.Direct3D11 || options.graphics === GraphicsApi_1.GraphicsApi.Direct3D12) {
            return 'd3d11';
        }
        else {
            throw new Error('Unsupported shader language.');
        }
    }
    watch(watch, match, options, recompileAll) {
        return new Promise((resolve, reject) => {
            let shaders = [];
            let ready = false;
            this.watcher = chokidar.watch(match, { ignored: /[\/\\]\.git/, persistent: watch });
            this.watcher.on('add', (filepath) => {
                let file = path.parse(filepath);
                if (ready) {
                    switch (file.ext) {
                        case '.glsl':
                            if (!file.name.endsWith('.inc')) {
                                log.info('Compiling ' + file.name);
                                this.compileShader(filepath, options, recompileAll);
                            }
                            break;
                    }
                }
                else {
                    switch (file.ext) {
                        case '.glsl':
                            if (!file.name.endsWith('.inc')) {
                                shaders.push(filepath);
                            }
                            break;
                    }
                }
            });
            this.watcher.on('unlink', (file) => {
            });
            this.watcher.on('ready', async () => {
                ready = true;
                let compiledShaders = [];
                const self = this;
                async function compile(shader, index) {
                    let parsed = path.parse(shader);
                    log.info('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ').');
                    let compiledShader = null;
                    try {
                        compiledShader = await self.compileShader(shader, options, recompileAll);
                    }
                    catch (error) {
                        log.error('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ') failed:');
                        log.error(error);
                        return Promise.reject(error);
                    }
                    if (compiledShader === null) {
                        compiledShader = new CompiledShader();
                        compiledShader.noembed = options.noembed;
                        // mark variables as invalid, so they are loaded from previous compilation
                        compiledShader.files = null;
                    }
                    if (compiledShader.files != null && compiledShader.files.length === 0) {
                        // TODO: Remove when krafix has been recompiled everywhere
                        compiledShader.files.push(parsed.name + '.' + self.type);
                    }
                    compiledShader.name = AssetConverter_1.AssetConverter.createExportInfo(parsed, false, options, self.exporter.options.from).name;
                    compiledShaders.push(compiledShader);
                    ++index;
                    return Promise.resolve();
                }
                if (this.options.parallelAssetConversion !== 0) {
                    let todo = shaders.map((shader, index) => {
                        return async () => {
                            await compile(shader, index);
                        };
                    });
                    let processes = this.options.parallelAssetConversion === -1
                        ? require('os').cpus().length - 1
                        : this.options.parallelAssetConversion;
                    await Throttle.all(todo, {
                        maxInProgress: processes,
                    });
                }
                else {
                    let index = 0;
                    for (let shader of shaders) {
                        try {
                            await compile(shader, index);
                        }
                        catch (err) {
                            reject();
                            return;
                        }
                        index += 1;
                    }
                }
                resolve(compiledShaders);
                return;
            });
        });
    }
    async run(watch, recompileAll) {
        let shaders = [];
        for (let matcher of this.shaderMatchers) {
            shaders = shaders.concat(await this.watch(watch, matcher.match, matcher.options, recompileAll));
        }
        return shaders;
    }
    compileShader(file, options, recompile) {
        return new Promise((resolve, reject) => {
            if (!this.compiler)
                reject('No shader compiler found.');
            if (this.type === 'none') {
                resolve(new CompiledShader());
                return;
            }
            let fileinfo = path.parse(file);
            let from = file;
            let to = path.join(this.to, fileinfo.name + '.' + this.type);
            let temp = to + '.temp';
            fs.stat(from, (fromErr, fromStats) => {
                fs.stat(to, (toErr, toStats) => {
                    if (options.noprocessing) {
                        if (!toStats || toStats.mtime.getTime() < fromStats.mtime.getTime()) {
                            fs.copySync(from, to, { overwrite: true });
                        }
                        let compiledShader = new CompiledShader();
                        compiledShader.noembed = options.noembed;
                        resolve(compiledShader);
                        return;
                    }
                    fs.stat(this.compiler, (compErr, compStats) => {
                        if (!recompile && (fromErr || (!toErr && toStats.mtime.getTime() > fromStats.mtime.getTime() && toStats.mtime.getTime() > compStats.mtime.getTime()))) {
                            if (fromErr)
                                log.error('Shader compiler error: ' + fromErr);
                            resolve(null);
                        }
                        else {
                            let parameters = [this.type === 'hlsl' ? 'd3d9' : this.type, from, temp, this.temp, this.platform];
                            if (this.options.shaderversion) {
                                parameters.push('--version');
                                parameters.push(this.options.shaderversion);
                            }
                            if (options.defines) {
                                for (let define of options.defines) {
                                    parameters.push('-D' + define);
                                }
                            }
                            parameters[1] = path.resolve(parameters[1]);
                            parameters[2] = path.resolve(parameters[2]);
                            parameters[3] = path.resolve(parameters[3]);
                            let child = child_process.spawn(this.compiler, parameters);
                            child.stdout.on('data', (data) => {
                                log.info(data.toString());
                            });
                            let errorLine = '';
                            let newErrorLine = true;
                            let errorData = false;
                            let compiledShader = new CompiledShader();
                            compiledShader.noembed = options.noembed;
                            child.stderr.on('data', (data) => {
                                let str = data.toString();
                                for (let char of str) {
                                    if (char === '\n') {
                                        if (errorData) {

                                        }
                                        else {
                                            log.error(errorLine.trim());
                                        }
                                        errorLine = '';
                                        newErrorLine = true;
                                        errorData = false;
                                    }
                                    else if (newErrorLine && char === '#') {
                                        errorData = true;
                                        newErrorLine = false;
                                    }
                                    else {
                                        errorLine += char;
                                        newErrorLine = false;
                                    }
                                }
                            });
                            child.on('close', (code) => {
                                if (errorLine.trim().length > 0) {
                                    if (errorData) {

                                    }
                                    else {
                                        log.error(errorLine.trim());
                                    }
                                }
                                if (code === 0) {
                                    if (compiledShader.files === null || compiledShader.files.length === 0) {
                                        fs.renameSync(temp, to);
                                    }
                                    for (let file of compiledShader.files) {
                                        fs.renameSync(path.join(this.to, file + '.temp'), path.join(this.to, file));
                                    }
                                    resolve(compiledShader);
                                }
                                else {
                                    process.exitCode = 1;
                                    reject('Shader compiler error.');
                                }
                            });
                        }
                    });
                });
            });
        });
    }
}
exports.ShaderCompiler = ShaderCompiler;
//# sourceMappingURL=ShaderCompiler.js.map