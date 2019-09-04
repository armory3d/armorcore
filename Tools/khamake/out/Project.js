"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process = require("child_process");
const fs = require("fs");
const path = require("path");
const log = require("./log");
const ProjectFile_1 = require("./ProjectFile");
class Library {
}
exports.Library = Library;
class Target {
    constructor(baseTarget, backends) {
        this.baseTarget = baseTarget;
        this.backends = backends;
    }
}
exports.Target = Target;
function contains(main, sub) {
    main = path.resolve(main);
    sub = path.resolve(sub);
    if (process.platform === 'win32') {
        main = main.toLowerCase();
        sub = sub.toLowerCase();
    }
    return sub.indexOf(main) === 0 && sub.slice(main.length)[0] === path.sep;
}
class Project {
    constructor(name) {
        this.icon = null;
        this.name = name;
        this.version = '1.0';
        this.sources = [];
        this.defines = [];
        this.cdefines = [];
        this.parameters = [];
        this.scriptdir = Project.scriptdir;
        this.libraries = [];
        this.localLibraryPath = 'Libraries';
        this.assetMatchers = [];
        this.shaderMatchers = [];
        this.customTargets = new Map();
        this.stackSize = 0;
        this.windowOptions = {};
        this.targetOptions = {};
    }
    async addProject(projectDir) {
        let project = await ProjectFile_1.loadProject(projectDir, 'khafile.js', Project.platform);
        this.assetMatchers = this.assetMatchers.concat(project.assetMatchers);
        this.sources = this.sources.concat(project.sources);
        this.shaderMatchers = this.shaderMatchers.concat(project.shaderMatchers);
        this.defines = this.defines.concat(project.defines);
        this.cdefines = this.cdefines.concat(project.cdefines);
        this.parameters = this.parameters.concat(project.parameters);
        this.libraries = this.libraries.concat(project.libraries);
        for (let customTarget of project.customTargets.keys()) {
            this.customTargets.set(customTarget, project.customTargets.get(customTarget));
        }
        // windowOptions and targetOptions are ignored
    }
    unglob(str) {
        const globChars = ['\\@', '\\!', '\\+', '\\*', '\\?', '\\(', '\\[', '\\{', '\\)', '\\]', '\\}'];
        str = str.replace(/\\/g, '/');
        for (const char of globChars) {
            str = str.replace(new RegExp(char, 'g'), '\\' + char.substr(1));
        }
        return str;
    }
    /**
     * Add all assets matching the match glob relative to the directory containing the current khafile.
     * Asset types are infered from the file suffix.
     * Glob syntax is very simple, the most important patterns are * for anything and ** for anything across directories.
     */
    addAssets(match, options) {
        if (!options)
            options = {};
        if (!path.isAbsolute(match)) {
            let base = this.unglob(path.resolve(this.scriptdir));
            if (!base.endsWith('/')) {
                base += '/';
            }
            match = base + match.replace(/\\/g, '/');
        }
        this.assetMatchers.push({ match: match, options: options });
    }
    addSources(source) {
        this.sources.push(path.resolve(path.join(this.scriptdir, source)));
    }
    /**
     * Add all shaders matching the match glob relative to the directory containing the current khafile.
     * Glob syntax is very simple, the most important patterns are * for anything and ** for anything across directories.
     */
    addShaders(match, options) {
        if (!options)
            options = {};
        if (!path.isAbsolute(match)) {
            let base = this.unglob(path.resolve(this.scriptdir));
            if (!base.endsWith('/')) {
                base += '/';
            }
            match = base + match.replace(/\\/g, '/');
        }
        this.shaderMatchers.push({ match: match, options: options });
    }
    addDefine(define) {
        this.defines.push(define);
    }
    addCDefine(define) {
        this.cdefines.push(define);
    }
    addParameter(parameter) {
        this.parameters.push(parameter);
    }
    addTarget(name, baseTarget, backends) {
        this.customTargets.set(name, new Target(baseTarget, backends));
    }
    addLibrary(library) {
        this.addDefine(library);
        let self = this;
        function findLibraryDirectory(name) {
            if (path.isAbsolute(name)) {
                return { libpath: name, libroot: name };
            }
            // check relative path
            if (fs.existsSync(path.resolve(name))) {
                return { libpath: name, libroot: name };
            }
            // Tries to load the default library from inside the kha project.
            // e.g. 'Libraries/wyngine'
            let libpath = path.join(self.scriptdir, self.localLibraryPath, name);
            if (fs.existsSync(libpath) && fs.statSync(libpath).isDirectory()) {
                return { libpath: path.resolve(libpath), libroot: self.localLibraryPath + '/' + name };
            }
            // Show error if library isn't found in Libraries or haxelib folder
            log.error('Error: Library ' + name + ' not found.');
            log.error('Add it to the \'Libraries\' subdirectory of your project.');
            throw 'Library ' + name + ' not found.';
        }
        let libInfo = findLibraryDirectory(library);
        let dir = libInfo.libpath;
        if (dir !== '') {
            this.libraries.push({
                libpath: dir,
                libroot: libInfo.libroot
            });
            this.sources.push(path.join(dir, 'Sources'));
            this.addShaders(dir + '/Sources/Shaders/**', {});
        }
    }
}
exports.Project = Project;
//# sourceMappingURL=Project.js.map