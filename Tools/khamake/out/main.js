"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const os = require("os");
const path = require("path");
const log = require("./log");
const ProjectFile_1 = require("./ProjectFile");
const AssetConverter_1 = require("./AssetConverter");
const HaxeCompiler_1 = require("./HaxeCompiler");
const ShaderCompiler_1 = require("./ShaderCompiler");
const KromExporter_1 = require("./KromExporter");
const HaxeProject_1 = require("./HaxeProject");
let lastAssetConverter;
let lastShaderCompiler;
let lastHaxeCompiler;
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
function fixName(name) {
    name = name.replace(/[-@\ \.\/\\]/g, '_');
    if (name[0] === '0' || name[0] === '1' || name[0] === '2' || name[0] === '3' || name[0] === '4'
        || name[0] === '5' || name[0] === '6' || name[0] === '7' || name[0] === '8' || name[0] === '9') {
        name = '_' + name;
    }
    return name;
}
function safeName(name) {
    return name.replace(/[\\\/]/g, '_');
}
async function exportProjectFiles(name, resourceDir, options, exporter, kore, korehl, icon, libraries, targetOptions, defines, cdefines, stackSize, version, id) {
    if (options.haxe !== '') {
        let haxeOptions = exporter.haxeOptions(name, targetOptions, defines);
        haxeOptions.defines.push('kha');
        haxeOptions.safeName = safeName(haxeOptions.name);
        HaxeProject_1.writeHaxeProject(options.to, !options.noproject, haxeOptions);
        
        let compiler = new HaxeCompiler_1.HaxeCompiler(options.to, haxeOptions.to, haxeOptions.realto, resourceDir, options.haxe, 'project-' + exporter.sysdir() + '.hxml', haxeOptions.sources, exporter.sysdir());
        lastHaxeCompiler = compiler;
        try {
            await compiler.run(false);
        }
        catch (error) {
            return Promise.reject(error);
        }
        
        for (let callback of ProjectFile_1.Callbacks.postHaxeCompilation) {
            callback();
        }
        await exporter.export(name, targetOptions, haxeOptions);
    }

    log.info('Done.');
    return name;
}
async function exportKhaProject(options) {
    log.info('Creating Kha project.');
    let project = null;
    let foundProjectFile = false;
    // get the khafile.js and load the config code,
    // then create the project config object, which contains stuff
    // like project name, assets paths, sources path, library path...
    if (fs.existsSync(path.join(options.from, 'khafile.js'))) {
        try {
            project = await ProjectFile_1.loadProject(options.from, 'khafile.js', options.target);
        }
        catch (x) {
            log.error(x);
            throw 'Loading the projectfile failed.';
        }
        foundProjectFile = true;
    }
    if (!foundProjectFile) {
        throw 'No khafile found.';
    }
    let temp = path.join(options.to, 'temp');
    fs.ensureDirSync(temp);
    let exporter = null;
    let kore = false;
    let korehl = false;
    let target = options.target.toLowerCase();
    let baseTarget = target;
    let customTarget = null;
    if (project.customTargets.get(options.target)) {
        customTarget = project.customTargets.get(options.target);
        baseTarget = customTarget.baseTarget;
    }
    exporter = new KromExporter_1.KromExporter(options);
    exporter.setSystemDirectory(target);
    let buildDir = path.join(options.to, exporter.sysdir() + '-build');
    // Create the target build folder
    // e.g. 'build/android-native'
    fs.ensureDirSync(path.join(options.to, exporter.sysdir()));
    exporter.setName(project.name);
    for (let source of project.sources) {
        exporter.addSourceDirectory(source);
    }
    for (let library of project.libraries) {
        exporter.addLibrary(library);
    }
    exporter.parameters = exporter.parameters.concat(project.parameters);
    project.scriptdir = options.kha;
    project.addShaders('Sources/Shaders/**', {});
    for (let callback of ProjectFile_1.Callbacks.preAssetConversion) {
        callback();
    }
    let assetConverter = new AssetConverter_1.AssetConverter(exporter, options, project.assetMatchers);
    lastAssetConverter = assetConverter;
    let assets = await assetConverter.run(false, temp);
    let shaderDir = path.join(options.to, exporter.sysdir() + '-resources');
    for (let callback of ProjectFile_1.Callbacks.preShaderCompilation) {
        callback();
    }
    fs.ensureDirSync(shaderDir);
    let oldResources = null;
    let recompileAllShaders = false;
    try {
        oldResources = JSON.parse(fs.readFileSync(path.join(options.to, exporter.sysdir() + '-resources', 'files.json'), 'utf8'));
        for (let file of oldResources.files) {
            if (file.type === 'shader') {
                if (!file.files || file.files.length === 0) {
                    recompileAllShaders = true;
                    break;
                }
            }
        }
    }
    catch (error) {
    }
    let exportedShaders = [];
    let shaderCompiler = new ShaderCompiler_1.ShaderCompiler(exporter, options.target, options.krafix, shaderDir, temp, buildDir, options, project.shaderMatchers);
    lastShaderCompiler = shaderCompiler;
    try {
        exportedShaders = await shaderCompiler.run(false, recompileAllShaders);
    }
    catch (err) {
        return Promise.reject(err);
    }
    
    function findShader(name) {
        let fallback = {};
        fallback.files = [];
        try {
            for (let file of oldResources.files) {
                if (file.type === 'shader' && file.name === fixName(name)) {
                    return file;
                }
            }
        }
        catch (error) {
            return fallback;
        }
        return fallback;
    }
    let files = [];
    for (let asset of assets) {
        let file = {
            name: fixName(asset.name),
            files: asset.files,
            type: asset.type
        };
        if (file.type === 'image') {
            file.original_width = asset.original_width;
            file.original_height = asset.original_height;
            if (asset.readable)
                file.readable = asset.readable;
        }
        files.push(file);
    }
    for (let shader of exportedShaders) {
        if (shader.noembed)
            continue;
        let oldShader = findShader(shader.name);
        files.push({
            name: fixName(shader.name),
            files: shader.files === null ? oldShader.files : shader.files,
            type: 'shader'
        });
    }
    // Sort to prevent files.json from changing between makes when no files have changed.
    files.sort(function (a, b) {
        if (a.name > b.name)
            return 1;
        if (a.name < b.name)
            return -1;
        return 0;
    });
    if (foundProjectFile) {
        fs.outputFileSync(path.join(options.to, exporter.sysdir() + '-resources', 'files.json'), JSON.stringify({ files: files }, null, '\t'));
    }
    for (let callback of ProjectFile_1.Callbacks.preHaxeCompilation) {
        callback();
    }
    
    return await exportProjectFiles(project.name, path.join(options.to, exporter.sysdir() + '-resources'), options, exporter, kore, korehl, project.icon, project.libraries, project.targetOptions, project.defines, project.cdefines, project.stackSize, project.version, project.id);
}
function isKhaProject(directory, projectfile) {
    return fs.existsSync(path.join(directory, 'Kha')) || fs.existsSync(path.join(directory, projectfile));
}
async function exportProject(options) {
    if (isKhaProject(options.from, 'khafile.js')) {
        return await exportKhaProject(options);
    }
    else {
        log.error('Neither Kha directory nor project file (' + 'khafile.js' + ') found.');
        return 'Unknown';
    }
}
exports.api = 2;
async function run(options, loglog) {
    options.target = 'krom';
    log.set(loglog);
    let p = path.join(__dirname, '..', '..', '..');
    if (fs.existsSync(p) && fs.statSync(p).isDirectory()) {
        options.kha = p;
    }
    log.info('Using Kha from ' + options.kha);
    if (options.parallelAssetConversion === undefined) {
        options.parallelAssetConversion = 0;
    }
    let haxepath = path.join(options.kha, 'Tools', 'haxe');
    if (fs.existsSync(haxepath) && fs.statSync(haxepath).isDirectory())
        options.haxe = haxepath;
    let krafixpath = path.join(options.kha, 'Kinc', 'Tools', 'krafix', 'krafix' + exec_sys());
    if (fs.existsSync(krafixpath))
        options.krafix = krafixpath;
    
    if (options.ffmpeg) {
        options.ogg = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.mp3 = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.aac = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.h264 = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.webm = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.wmv = options.ffmpeg + ' -nostdin -i {in} {out}';
        options.theora = options.ffmpeg + ' -nostdin -i {in} {out}';
    }
    if (!options.ogg) {
        let oggpath = path.join(options.kha, 'Tools', 'oggenc', 'oggenc' + exec_sys());
        if (fs.existsSync(oggpath))
            options.ogg = oggpath + ' {in} -o {out} --quiet';
    }
    if (!options.mp3) {
        let lamepath = path.join(options.kha, 'Tools', 'lame', 'lame' + exec_sys());
        if (fs.existsSync(lamepath))
            options.mp3 = lamepath + ' {in} {out}';
    }
    let name = '';
    try {
        name = await exportProject(options);
    }
    catch (err) {
        process.exit(1);
    }
    return name;
}
exports.run = run;
function close() {
    if (lastAssetConverter)
        lastAssetConverter.close();
    if (lastShaderCompiler)
        lastShaderCompiler.close();
    if (lastHaxeCompiler)
        lastHaxeCompiler.close();
}
exports.close = close;
//# sourceMappingURL=main.js.map