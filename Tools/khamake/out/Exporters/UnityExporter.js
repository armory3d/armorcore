"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs-extra");
const path = require("path");
const KhaExporter_1 = require("./KhaExporter");
const Converter_1 = require("../Converter");
const ImageTool_1 = require("../ImageTool");
const uuid = require('uuid');
class UnityExporter extends KhaExporter_1.KhaExporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'Unity';
    }
    haxeOptions(name, targetOptions, defines) {
        const sources = path.join(this.options.to, this.sysdir(), 'Assets', 'Sources');
        if (fs.existsSync(sources)) {
            fs.removeSync(sources);
        }
        defines.push('no-root');
        defines.push('no-compilation');
        defines.push('sys_' + this.options.target);
        defines.push('sys_g1');
        defines.push('sys_g2');
        defines.push('sys_g3');
        defines.push('sys_g4');
        defines.push('sys_a1');
        defines.push('kha_cs');
        defines.push('kha_' + this.options.target);
        defines.push('kha_' + this.options.target + '_cs');
        defines.push('kha_g1');
        defines.push('kha_g2');
        defines.push('kha_g3');
        defines.push('kha_g4');
        defines.push('kha_a1');
        return {
            from: this.options.from,
            to: path.join(this.sysdir(), 'Assets', 'Sources'),
            sources: this.sources,
            libraries: this.libraries,
            defines: defines,
            parameters: this.parameters,
            haxeDirectory: this.options.haxe,
            system: this.sysdir(),
            language: 'cs',
            width: this.width,
            height: this.height,
            name: name,
            main: this.options.main,
        };
    }
    async export(name, targetOptions, haxeOptions) {
        let copyDirectory = (from, to) => {
            let files = fs.readdirSync(path.join(__dirname, '..', '..', 'Data', 'unity', from));
            fs.ensureDirSync(path.join(this.options.to, this.sysdir(), to));
            for (let file of files) {
                let text = fs.readFileSync(path.join(__dirname, '..', '..', 'Data', 'unity', from, file), 'utf8');
                fs.writeFileSync(path.join(this.options.to, this.sysdir(), to, file), text);
            }
        };
        copyDirectory('Assets', 'Assets');
        copyDirectory('Editor', 'Assets/Editor');
        copyDirectory('ProjectSettings', 'ProjectSettings');
    }
    /*copyMusic(platform, from, to, encoders, callback) {
        callback([to]);
    }*/
    async copySound(platform, from, to) {
        let ogg = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), 'Assets', 'Resources', 'Sounds', to + '.ogg'), this.options.ogg);
        return [to + '.ogg'];
    }
    async copyImage(platform, from, to, asset, cache) {
        let format = await ImageTool_1.exportImage(this.options.kha, from, path.join(this.options.to, this.sysdir(), 'Assets', 'Resources', 'Images', to), asset, undefined, false, true, cache);
        return [to + '.' + format];
    }
    async copyBlob(platform, from, to) {
        fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), 'Assets', 'Resources', 'Blobs', to + '.bytes'), { overwrite: true });
        return [to];
    }
    async copyVideo(platform, from, to) {
        return [to];
    }
}
exports.UnityExporter = UnityExporter;
//# sourceMappingURL=UnityExporter.js.map