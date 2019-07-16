"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const os = require("os");
const path = require("path");
const chokidar = require("chokidar");
const Throttle = require("promise-parallel-throttle");
const GraphicsApi_1 = require("./GraphicsApi");
const Platform_1 = require("./Platform");
const AssetConverter_1 = require("./AssetConverter");
const log = require("./log");
class CompiledShader {
    constructor() {
        this.files = [];
        this.inputs = [];
        this.outputs = [];
        this.uniforms = [];
        this.types = [];
        this.noembed = false;
    }
}
exports.CompiledShader = CompiledShader;
class ShaderCompiler {
    constructor(exporter, platform, compiler, to, temp, builddir, options, shaderMatchers) {
        this.exporter = exporter;
        if (platform.endsWith('-native'))
            platform = platform.substr(0, platform.length - '-native'.length);
        if (platform.endsWith('-hl'))
            platform = platform.substr(0, platform.length - '-hl'.length);
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
        switch (platform) {
            case Platform_1.Platform.Empty:
            case Platform_1.Platform.Node:
                return 'glsl';
            case Platform_1.Platform.Flash:
                return 'agal';
            case Platform_1.Platform.Android:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Vulkan) {
                    return 'spirv';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL || options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    return 'essl';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            case Platform_1.Platform.HTML5:
            case Platform_1.Platform.DebugHTML5:
            case Platform_1.Platform.HTML5Worker:
            case Platform_1.Platform.Tizen:
            case Platform_1.Platform.Pi:
                return 'essl';
            case Platform_1.Platform.tvOS:
            case Platform_1.Platform.iOS:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Metal || options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    return 'metal';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL) {
                    return 'essl';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            case Platform_1.Platform.Windows:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Vulkan) {
                    return 'spirv';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL) {
                    return 'glsl';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Direct3D11 || options.graphics === GraphicsApi_1.GraphicsApi.Direct3D12 || options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    return 'd3d11';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Direct3D9) {
                    return 'd3d9';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            case Platform_1.Platform.WindowsApp:
                return 'd3d11';
            case Platform_1.Platform.Xbox360:
            case Platform_1.Platform.PlayStation3:
                return 'd3d9';
            case Platform_1.Platform.Linux:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Vulkan) {
                    return 'spirv';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL || options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    return 'glsl';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            case Platform_1.Platform.OSX:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Metal || options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    return 'metal';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL) {
                    return 'glsl';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            case Platform_1.Platform.Unity:
                return 'hlsl';
            case Platform_1.Platform.Krom:
                if (options.graphics === GraphicsApi_1.GraphicsApi.Default) {
                    if (process.platform === 'win32') {
                        return 'd3d11';
                    }
                    else if (process.platform === 'darwin') {
                        return 'metal';
                    }
                    else {
                        return 'glsl';
                    }
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Vulkan) {
                    return 'spirv';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Metal) {
                    return 'metal';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.OpenGL) {
                    return 'glsl';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Direct3D11 || options.graphics === GraphicsApi_1.GraphicsApi.Direct3D12) {
                    return 'd3d11';
                }
                else if (options.graphics === GraphicsApi_1.GraphicsApi.Direct3D9) {
                    return 'd3d9';
                }
                else {
                    throw new Error('Unsupported shader language.');
                }
            default:
                return platform;
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
            if (watch) {
                this.watcher.on('change', (filepath) => {
                    let file = path.parse(filepath);
                    switch (file.ext) {
                        case '.glsl':
                            if (!file.name.endsWith('.inc')) {
                                log.info('Recompiling ' + file.name);
                                this.compileShader(filepath, options, recompileAll);
                            }
                            break;
                    }
                });
            }
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
                        compiledShader.inputs = null;
                        compiledShader.outputs = null;
                        compiledShader.uniforms = null;
                        compiledShader.types = null;
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
                            if (this.type === 'metal' && this.platform !== Platform_1.Platform.Krom) {
                                fs.ensureDirSync(path.join(this.builddir, 'Sources'));
                                let funcname = fileinfo.name;
                                funcname = funcname.replace(/-/g, '_');
                                funcname = funcname.replace(/\./g, '_');
                                funcname += '_main';
                                fs.writeFileSync(to, '>' + funcname, 'utf8');
                                to = path.join(this.builddir, 'Sources', fileinfo.name + '.' + this.type);
                                temp = to;
                            }
                            let parameters = [this.type === 'hlsl' ? 'd3d9' : this.type, from, temp, this.temp, this.platform];
                            if (this.options.shaderversion) {
                                parameters.push('--version');
                                parameters.push(this.options.shaderversion);
                            }
                            else if (this.platform === Platform_1.Platform.Krom && os.platform() === 'linux') {
                                parameters.push('--version');
                                parameters.push('110');
                            }
                            if (this.options.glsl2) {
                                parameters.push('--glsl2');
                            }
                            if (options.defines) {
                                for (let define of options.defines) {
                                    parameters.push('-D' + define);
                                }
                            }
                            if (this.platform === Platform_1.Platform.HTML5 || this.platform === Platform_1.Platform.HTML5Worker || this.platform === Platform_1.Platform.Android) {
                                parameters.push('--relax');
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
                            function parseData(data) {
                                data = data.replace(':\\', '#\\'); // Filter out absolute paths on Windows
                                let parts = data.split(':');
                                if (parts.length >= 3) {
                                    if (parts[0] === 'uniform') {
                                        compiledShader.uniforms.push({ name: parts[1], type: parts[2] });
                                    }
                                    else if (parts[0] === 'input') {
                                        compiledShader.inputs.push({ name: parts[1], type: parts[2] });
                                    }
                                    else if (parts[0] === 'output') {
                                        compiledShader.outputs.push({ name: parts[1], type: parts[2] });
                                    }
                                    else if (parts[0] === 'type') {
                                        let type = data.substring(data.indexOf(':') + 1);
                                        let name = type.substring(0, type.indexOf(':'));
                                        let typedata = type.substring(type.indexOf(':') + 2);
                                        typedata = typedata.substr(0, typedata.length - 1);
                                        let members = typedata.split(',');
                                        let memberdecls = [];
                                        for (let member of members) {
                                            let memberparts = member.split(':');
                                            memberdecls.push({ type: memberparts[1], name: memberparts[0] });
                                        }
                                        compiledShader.types.push({ name: name, members: memberdecls });
                                    }
                                }
                                else if (parts.length >= 2) {
                                    if (parts[0] === 'file') {
                                        const parsed = path.parse(parts[1].replace('#\\', ':\\'));
                                        let name = parsed.name;
                                        if (parsed.ext !== '.temp')
                                            name += parsed.ext;
                                        compiledShader.files.push(name);
                                    }
                                }
                            }
                            child.stderr.on('data', (data) => {
                                let str = data.toString();
                                for (let char of str) {
                                    if (char === '\n') {
                                        if (errorData) {
                                            parseData(errorLine.trim());
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
                                        parseData(errorLine.trim());
                                    }
                                    else {
                                        log.error(errorLine.trim());
                                    }
                                }
                                if (code === 0) {
                                    if (this.type !== 'metal' || this.platform === Platform_1.Platform.Krom) {
                                        if (compiledShader.files === null || compiledShader.files.length === 0) {
                                            fs.renameSync(temp, to);
                                        }
                                        for (let file of compiledShader.files) {
                                            fs.renameSync(path.join(this.to, file + '.temp'), path.join(this.to, file));
                                        }
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