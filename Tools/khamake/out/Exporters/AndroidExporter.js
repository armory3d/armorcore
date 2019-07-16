"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs-extra");
const path = require("path");
const KhaExporter_1 = require("./KhaExporter");
const Converter_1 = require("../Converter");
const ImageTool_1 = require("../ImageTool");
function findIcon(from, options) {
    if (fs.existsSync(path.join(from, 'icon.png')))
        return path.join(from, 'icon.png');
    else
        return path.join(options.kha, 'Kinc', 'Tools', 'kraffiti', 'ball.png');
}
class AndroidExporter extends KhaExporter_1.KhaExporter {
    constructor(options) {
        super(options);
    }
    backend() {
        return 'Android';
    }
    haxeOptions(name, targetOptions, defines) {
        const safename = name.replace(/ /g, '-');
        defines.push('no-compilation');
        defines.push('sys_' + this.options.target);
        defines.push('sys_g1');
        defines.push('sys_g2');
        defines.push('sys_g3');
        defines.push('sys_g4');
        defines.push('sys_a1');
        defines.push('kha_java');
        defines.push('kha_' + this.options.target);
        defines.push('kha_' + this.options.target + '_java');
        defines.push('kha_opengl');
        defines.push('kha_g1');
        defines.push('kha_g2');
        defines.push('kha_g3');
        defines.push('kha_g4');
        defines.push('kha_a1');
        return {
            from: this.options.from,
            to: path.join(this.sysdir(), safename),
            sources: this.sources,
            libraries: this.libraries,
            defines: defines,
            parameters: this.parameters,
            haxeDirectory: this.options.haxe,
            system: this.sysdir(),
            language: 'java',
            width: this.width,
            height: this.height,
            name: name,
            main: this.options.main,
        };
    }
    async export(name, targetOptions, haxeOptions) {
        if (this.projectFiles) {
            this.exportAndroidStudioProject(name, targetOptions, this.options.from);
        }
    }
    exportAndroidStudioProject(name, _targetOptions, from) {
        let safename = name.replace(/ /g, '-');
        this.safename = safename;
        let targetOptions = {
            package: 'tech.kode.kha',
            installLocation: 'internalOnly',
            screenOrientation: 'sensor',
            permissions: new Array()
        };
        if (_targetOptions != null && _targetOptions.android != null) {
            let userOptions = _targetOptions.android;
            if (userOptions.package != null)
                targetOptions.package = userOptions.package;
            if (userOptions.installLocation != null)
                targetOptions.installLocation = userOptions.installLocation;
            if (userOptions.screenOrientation != null)
                targetOptions.screenOrientation = userOptions.screenOrientation;
            if (userOptions.permissions != null)
                targetOptions.permissions = userOptions.permissions;
        }
        let indir = path.join(__dirname, '..', '..', 'Data', 'android');
        let outdir = path.join(this.options.to, this.sysdir(), safename);
        fs.copySync(path.join(indir, 'gitignore'), path.join(outdir, '.gitignore'));
        fs.copySync(path.join(indir, 'build.gradle'), path.join(outdir, 'build.gradle'));
        fs.copySync(path.join(indir, 'gradle.properties'), path.join(outdir, 'gradle.properties'));
        fs.copySync(path.join(indir, 'gradlew'), path.join(outdir, 'gradlew'));
        fs.copySync(path.join(indir, 'gradlew.bat'), path.join(outdir, 'gradlew.bat'));
        fs.copySync(path.join(indir, 'settings.gradle'), path.join(outdir, 'settings.gradle'));
        fs.copySync(path.join(indir, 'app', 'gitignore'), path.join(outdir, 'app', '.gitignore'));
        let gradle = fs.readFileSync(path.join(indir, 'app', 'build.gradle'), { encoding: 'utf8' });
        gradle = gradle.replace(/{package}/g, targetOptions.package);
        fs.writeFileSync(path.join(outdir, 'app', 'build.gradle'), gradle, { encoding: 'utf8' });
        fs.copySync(path.join(indir, 'app', 'proguard-rules.pro'), path.join(outdir, 'app', 'proguard-rules.pro'));
        fs.ensureDirSync(path.join(outdir, 'app', 'src'));
        // fs.emptyDirSync(path.join(outdir, 'app', 'src'));
        let manifest = fs.readFileSync(path.join(indir, 'main', 'AndroidManifest.xml'), { encoding: 'utf8' });
        manifest = manifest.replace(/{package}/g, targetOptions.package);
        manifest = manifest.replace(/{installLocation}/g, targetOptions.installLocation);
        manifest = manifest.replace(/{screenOrientation}/g, targetOptions.screenOrientation);
        manifest = manifest.replace(/{permissions}/g, targetOptions.permissions.map(function (p) { return '\n\t<uses-permission android:name="' + p + '"/>'; }).join(''));
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main'));
        fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'AndroidManifest.xml'), manifest, { encoding: 'utf8' });
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values'));
        let strings = fs.readFileSync(path.join(indir, 'main', 'res', 'values', 'strings.xml'), { encoding: 'utf8' });
        strings = strings.replace(/{name}/g, name);
        fs.writeFileSync(path.join(outdir, 'app', 'src', 'main', 'res', 'values', 'strings.xml'), strings, { encoding: 'utf8' });
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-hdpi'));
        ImageTool_1.exportImage(this.options.kha, findIcon(from, this.options), path.join(this.options.to, this.sysdir(), safename, 'app', 'src', 'main', 'res', 'mipmap-hdpi', 'ic_launcher'), { width: 72, height: 72 }, 'png', false, false, {});
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-mdpi'));
        ImageTool_1.exportImage(this.options.kha, findIcon(from, this.options), path.join(this.options.to, this.sysdir(), safename, 'app', 'src', 'main', 'res', 'mipmap-mdpi', 'ic_launcher'), { width: 48, height: 48 }, 'png', false, false, {});
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xhdpi'));
        ImageTool_1.exportImage(this.options.kha, findIcon(from, this.options), path.join(this.options.to, this.sysdir(), safename, 'app', 'src', 'main', 'res', 'mipmap-xhdpi', 'ic_launcher'), { width: 96, height: 96 }, 'png', false, false, {});
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi'));
        ImageTool_1.exportImage(this.options.kha, findIcon(from, this.options), path.join(this.options.to, this.sysdir(), safename, 'app', 'src', 'main', 'res', 'mipmap-xxhdpi', 'ic_launcher'), { width: 144, height: 144 }, 'png', false, false, {});
        fs.ensureDirSync(path.join(outdir, 'app', 'src', 'main', 'res', 'mipmap-xxxhdpi'));
        ImageTool_1.exportImage(this.options.kha, findIcon(from, this.options), path.join(this.options.to, this.sysdir(), safename, 'app', 'src', 'main', 'res', 'mipmap-xxxhdpi', 'ic_launcher'), { width: 192, height: 192 }, 'png', false, false, {});
        fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.jar'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.jar'));
        fs.copySync(path.join(indir, 'gradle', 'wrapper', 'gradle-wrapper.properties'), path.join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.properties'));
        fs.copySync(path.join(indir, 'idea', 'gradle.xml'), path.join(outdir, '.idea', 'gradle.xml'));
        fs.copySync(path.join(indir, 'idea', 'misc.xml'), path.join(outdir, '.idea', 'misc.xml'));
        fs.copySync(path.join(indir, 'idea', 'runConfigurations.xml'), path.join(outdir, '.idea', 'runConfigurations.xml'));
        fs.copySync(path.join(indir, 'idea', 'codeStyles', 'Project.xml'), path.join(outdir, '.idea', 'codeStyles', 'Project.xml'));
    }
    /*copyMusic(platform, from, to, encoders, callback) {
        Files.createDirectories(this.directory.resolve(this.sysdir()).resolve(to).parent());
        Converter.convert(from, this.directory.resolve(Paths.get(this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', to + '.ogg')), encoders.oggEncoder, function (success) {
            callback([to + '.ogg']);
        });
    }*/
    async copySound(platform, from, to, options) {
        if (options.quality < 1) {
            fs.ensureDirSync(path.join(this.options.to, this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', path.dirname(to)));
            let ogg = await Converter_1.convert(from, path.join(this.options.to, this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', to + '.ogg'), this.options.ogg);
            return [to + '.ogg'];
        }
        else {
            fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', to + '.wav'), { overwrite: true });
            return [to + '.wav'];
        }
    }
    async copyImage(platform, from, to, asset, cache) {
        let format = await ImageTool_1.exportImage(this.options.kha, from, path.join(this.options.to, this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', to), asset, undefined, false, false, cache);
        return [to + '.' + format];
    }
    async copyBlob(platform, from, to) {
        fs.copySync(from.toString(), path.join(this.options.to, this.sysdir(), this.safename, 'app', 'src', 'main', 'assets', to), { overwrite: true });
        return [to];
    }
    async copyVideo(platform, from, to) {
        return [to];
    }
}
exports.AndroidExporter = AndroidExporter;
//# sourceMappingURL=AndroidExporter.js.map