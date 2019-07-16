"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs-extra");
const path = require("path");
const KhaExporter_1 = require("./KhaExporter");
const Converter_1 = require("../Converter");
const ImageTool_1 = require("../ImageTool");
const VrApi_1 = require("../VrApi");
class Html5Exporter extends KhaExporter_1.KhaExporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'HTML5';
    }
    isDebugHtml5() {
        return this.sysdir() === 'debug-html5';
    }
    isNode() {
        return this.sysdir() === 'node';
    }
    isHtml5Worker() {
        return this.sysdir() === 'html5worker';
    }
    haxeOptions(name, targetOptions, defines) {
        defines.push('sys_g1');
        defines.push('sys_g2');
        defines.push('sys_g3');
        defines.push('sys_a1');
        defines.push('sys_a2');
        defines.push('kha_js');
        defines.push('kha_g1');
        defines.push('kha_g2');
        defines.push('kha_g3');
        defines.push('kha_a1');
        defines.push('kha_a2');
        if (targetOptions.html5.noKeyboard) {
            defines.push('kha_no_keyboard');
        }
        if (targetOptions.html5.disableContextMenu) {
            defines.push('kha_disable_context_menu');
        }
        if (this.options.vr === VrApi_1.VrApi.WebVR) {
            defines.push('kha_webvr');
        }
        let canvasId = targetOptions.html5.canvasId == null ? 'khanvas' : targetOptions.html5.canvasId;
        defines.push('canvas_id=' + canvasId);
        let scriptName = this.isHtml5Worker() ? 'khaworker' : 'kha';
        if (targetOptions.html5.scriptName != null && !(this.isNode() || this.isDebugHtml5())) {
            scriptName = targetOptions.html5.scriptName;
        }
        defines.push('script_name=' + scriptName);
        let webgl = targetOptions.html5.webgl == null ? true : targetOptions.html5.webgl;
        if (webgl) {
            defines.push('sys_g4');
            defines.push('kha_g4');
            defines.push('kha_webgl');
        }
        else {
            defines.push('kha_html5_canvas');
        }
        if (this.isNode()) {
            defines.push('nodejs');
            defines.push('sys_node');
            defines.push('sys_server');
            defines.push('kha_node');
            defines.push('kha_server');
        }
        else {
            defines.push('sys_' + this.options.target);
            defines.push('kha_' + this.options.target);
            defines.push('kha_' + this.options.target + '_js');
        }
        if (this.isDebugHtml5()) {
            this.parameters.push('-debug');
            defines.push('sys_debug_html5');
            defines.push('kha_debug_html5');
            defines.push('kha_html5');
        }
        if (this.isHtml5Worker()) {
            defines.push('js-classic');
        }
        return {
            from: this.options.from.toString(),
            to: path.join(this.sysdir(), scriptName + '.js'),
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
    async export(name, _targetOptions, haxeOptions) {
        let targetOptions = {
            canvasId: 'khanvas',
            scriptName: this.isHtml5Worker() ? 'khaworker' : 'kha'
        };
        if (_targetOptions != null && _targetOptions.html5 != null) {
            let userOptions = _targetOptions.html5;
            if (userOptions.canvasId != null)
                targetOptions.canvasId = userOptions.canvasId;
            if (userOptions.scriptName != null)
                targetOptions.scriptName = userOptions.scriptName;
        }
        fs.ensureDirSync(path.join(this.options.to, this.sysdir()));
        if (this.isDebugHtml5()) {
            let index = path.join(this.options.to, this.sysdir(), 'index.html');
            let protoindex = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'debug-html5', 'index.html'), { encoding: 'utf8' });
            protoindex = protoindex.replace(/{Name}/g, name);
            protoindex = protoindex.replace(/{Width}/g, '' + this.width);
            protoindex = protoindex.replace(/{Height}/g, '' + this.height);
            protoindex = protoindex.replace(/{CanvasId}/g, '' + targetOptions.canvasId);
            protoindex = protoindex.replace(/{ScriptName}/g, '' + targetOptions.scriptName);
            fs.writeFileSync(index.toString(), protoindex);
            let pack = path.join(this.options.to, this.sysdir(), 'package.json');
            let protopackage = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'debug-html5', 'package.json'), { encoding: 'utf8' });
            protopackage = protopackage.replace(/{Name}/g, name);
            fs.writeFileSync(pack.toString(), protopackage);
            let electron = path.join(this.options.to, this.sysdir(), 'electron.js');
            let protoelectron = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'debug-html5', 'electron.js'), { encoding: 'utf8' });
            protoelectron = protoelectron.replace(/{Width}/g, '' + this.width);
            protoelectron = protoelectron.replace(/{Height}/g, '' + this.height);
            fs.writeFileSync(electron.toString(), protoelectron);
        }
        else if (this.isNode()) {
            let pack = path.join(this.options.to, this.sysdir(), 'package.json');
            let protopackage = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'node', 'package.json'), 'utf8');
            protopackage = protopackage.replace(/{Name}/g, name);
            fs.writeFileSync(pack, protopackage);
            let protoserver = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'node', 'server.js'), 'utf8');
            fs.writeFileSync(path.join(this.options.to, this.sysdir(), 'server.js'), protoserver);
        }
        else if (!this.isHtml5Worker()) {
            let index = path.join(this.options.to, this.sysdir(), 'index.html');
            if (!fs.existsSync(index)) {
                let protoindex = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'html5', 'index.html'), { encoding: 'utf8' });
                protoindex = protoindex.replace(/{Name}/g, name);
                protoindex = protoindex.replace(/{Width}/g, '' + this.width);
                protoindex = protoindex.replace(/{Height}/g, '' + this.height);
                protoindex = protoindex.replace(/{CanvasId}/g, '' + targetOptions.canvasId);
                protoindex = protoindex.replace(/{ScriptName}/g, '' + targetOptions.scriptName);
                fs.writeFileSync(index.toString(), protoindex);
            }
        }
    }
    /*copyMusic(platform, from, to, encoders, callback) {
        Files.createDirectories(this.directory.resolve(this.sysdir()).resolve(to).parent());
        Converter.convert(from, this.directory.resolve(this.sysdir()).resolve(to + '.ogg'), encoders.oggEncoder, (ogg) => {
            Converter.convert(from, this.directory.resolve(this.sysdir()).resolve(to + '.mp4'), encoders.aacEncoder, (mp4) => {
                var files = [];
                if (ogg) files.push(to + '.ogg');
                if (mp4) files.push(to + '.mp4');
                callback(files);
            });
        });
    }*/
    async copySound(platform, from, to, options) {
        fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
        let ogg = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), to + '.ogg'), this.options.ogg);
        let mp4 = false;
        let mp3 = false;
        if (!this.isDebugHtml5()) {
            mp4 = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), to + '.mp4'), this.options.aac);
            if (!mp4) {
                mp3 = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), to + '.mp3'), this.options.mp3);
            }
        }
        let files = [];
        if (ogg)
            files.push(to + '.ogg');
        if (mp4)
            files.push(to + '.mp4');
        if (mp3)
            files.push(to + '.mp3');
        return files;
    }
    async copyImage(platform, from, to, options, cache) {
        let format = await ImageTool_1.exportImage(this.options.kha, from, path.join(this.options.to, this.sysdir(), to), options, undefined, false, false, cache);
        return [to + '.' + format];
    }
    async copyBlob(platform, from, to, options) {
        fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), to), { overwrite: true });
        return [to];
    }
    async copyVideo(platform, from, to, options) {
        fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
        let mp4 = false;
        if (!this.isDebugHtml5()) {
            mp4 = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), to + '.mp4'), this.options.h264);
        }
        let webm = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), to + '.webm'), this.options.webm);
        let files = [];
        if (mp4)
            files.push(to + '.mp4');
        if (webm)
            files.push(to + '.webm');
        return files;
    }
}
exports.Html5Exporter = Html5Exporter;
//# sourceMappingURL=Html5Exporter.js.map