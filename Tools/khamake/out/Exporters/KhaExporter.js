"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const path = require("path");
const Exporter_1 = require("./Exporter");
class KhaExporter extends Exporter_1.Exporter {
    constructor(options) {
        super();
        this.options = options;
        this.width = 640;
        this.height = 480;
        this.sources = [];
        this.libraries = [];
        this.addSourceDirectory(path.join(options.kha, 'Sources'));
        this.projectFiles = !options.noproject;
        this.parameters = [];
        // this.parameters = ['--macro kha.internal.GraphicsBuilder.build("' + this.backend().toLowerCase() + '")'];
        this.addSourceDirectory(path.join(options.kha, 'Backends', this.backend()));
    }
    sysdir() {
        return this.systemDirectory;
    }
    setWidthAndHeight(width, height) {
        this.width = width;
        this.height = height;
    }
    setName(name) {
        this.name = name;
        this.safename = name.replace(/ /g, '-');
    }
    setSystemDirectory(systemDirectory) {
        this.systemDirectory = systemDirectory;
    }
    addShader(shader) {
    }
    addSourceDirectory(path) {
        this.sources.push(path);
    }
    addLibrary(library) {
        this.libraries.push(library);
    }
    removeSourceDirectory(path) {
        for (let i = 0; i < this.sources.length; ++i) {
            if (this.sources[i] === path) {
                this.sources.splice(i, 1);
                return;
            }
        }
    }
    async copyImage(platform, from, to, options, cache) {
        return [];
    }
    async copySound(platform, from, to, options) {
        return [];
    }
    async copyVideo(platform, from, to, options) {
        return [];
    }
    async copyBlob(platform, from, to, options) {
        return [];
    }
    async copyFont(platform, from, to, options) {
        return await this.copyBlob(platform, from, to + '.ttf', options);
    }
}
exports.KhaExporter = KhaExporter;
//# sourceMappingURL=KhaExporter.js.map