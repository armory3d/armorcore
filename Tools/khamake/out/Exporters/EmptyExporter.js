"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs-extra");
const path = require("path");
const KhaExporter_1 = require("./KhaExporter");
const Haxe_1 = require("../Haxe");
const log = require("../log");
class EmptyExporter extends KhaExporter_1.KhaExporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'Empty';
    }
    haxeOptions(name, targetOptions, defines) {
        defines.push('sys_g1');
        defines.push('sys_g2');
        defines.push('sys_g3');
        defines.push('sys_g4');
        defines.push('sys_a1');
        defines.push('sys_a2');
        defines.push('kha_g1');
        defines.push('kha_g2');
        defines.push('kha_g3');
        defines.push('kha_g4');
        defines.push('kha_a1');
        defines.push('kha_a2');
        return {
            from: this.options.from,
            to: path.join(this.sysdir(), 'docs.xml'),
            sources: this.sources,
            libraries: this.libraries,
            defines: defines,
            parameters: this.parameters,
            haxeDirectory: this.options.haxe,
            system: this.sysdir(),
            language: 'xml',
            width: this.width,
            height: this.height,
            name: name,
            main: this.options.main,
        };
    }
    async export(name, _targetOptions, haxeOptions) {
        fs.ensureDirSync(path.join(this.options.to, this.sysdir()));
        try {
            // Remove any @:export first
            await Haxe_1.executeHaxe(this.options.to, this.options.haxe, ['project-' + this.sysdir() + '.hxml']);
            let doxresult = child_process.spawnSync('haxelib', ['run', 'dox', '-in', 'kha.*', '-i', path.join('build', this.sysdir(), 'docs.xml')], { env: process.env, cwd: path.normalize(this.options.from) });
            if (doxresult.stdout.toString() !== '') {
                log.info(doxresult.stdout.toString());
            }
            if (doxresult.stderr.toString() !== '') {
                log.error(doxresult.stderr.toString());
            }
        }
        catch (error) {
        }
    }
    async copySound(platform, from, to) {
        return [''];
    }
    async copyImage(platform, from, to, asset) {
        return [''];
    }
    async copyBlob(platform, from, to) {
        return [''];
    }
    async copyVideo(platform, from, to) {
        return [''];
    }
}
exports.EmptyExporter = EmptyExporter;
//# sourceMappingURL=EmptyExporter.js.map