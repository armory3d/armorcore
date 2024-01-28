const fs = require("fs");
const os = require("os");
const path = require("path");
const child_process = require("child_process");

function ensureDirSync(dir) {
	const parent = path.normalize(path.join(dir, '..'));
	if (!(0, fs.existsSync)(parent)) {
		ensureDirSync(parent);
	}
	if (!(0, fs.existsSync)(dir)) {
		(0, fs.mkdirSync)(dir);
	}
}
fs.ensureDirSync = ensureDirSync;

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

function sys() {
	if (os.platform() === 'win32') {
		return '.exe';
	}
	else {
		return '';
	}
}

function sysdir() {
	if (os.platform() === 'linux') {
		if (os.arch() === 'arm64') return 'linux_arm64';
		else if (os.arch() === 'x64') return 'linux_x64';
		else throw 'Unsupported CPU';
	}
	else if (os.platform() === 'win32') {
		return 'windows_x64';
	}
	else {
		return 'macos';
	}
}

function matches(text, pattern) {
	const regexstring = pattern.replace(/\./g, '\\.').replace(/\*\*/g, '.?').replace(/\*/g, '[^/]*').replace(/\?/g, '*');
	const regex = new RegExp('^' + regexstring + '$', 'g');
	return regex.test(text);
}

function stringify(path) {
	return path.replace(/\\/g, '/');
}

function searchFiles(currentDir, pattern) {
	let result = [];
	if (!fs.existsSync(currentDir)) {
		return result;
	}
	let files = fs.readdirSync(currentDir);
	for (let f in files) {
		let file = path.join(currentDir, files[f]);
		if (fs.statSync(file).isDirectory())
			continue;
		file = path.relative(currentDir, file);
		if (matches(stringify(file), stringify(pattern))) {
			result.push(path.join(currentDir, stringify(file)));
		}
	}
	if (pattern.endsWith("**")) {
		let dirs = fs.readdirSync(currentDir);
		for (let d of dirs) {
			let dir = path.join(currentDir, d);
			if (d.startsWith('.'))
				continue;
			if (!fs.statSync(dir).isDirectory())
				continue;
			result = result.concat(searchFiles(dir, pattern));
		}
	}
	return result;
}

class Project {
	constructor(name) {
		this.name = name;
		this.sources = [];
		this.defines = [];
		this.parameters = [];
		this.scriptdir = Project.scriptdir;
		this.libraries = [];
		this.assetMatchers = [];
		this.shaderMatchers = [];
	}

	async addProject(projectDir) {
		let project = await loadProject(projectDir, 'project.js');
		this.assetMatchers = this.assetMatchers.concat(project.assetMatchers);
		this.sources = this.sources.concat(project.sources);
		this.shaderMatchers = this.shaderMatchers.concat(project.shaderMatchers);
		this.defines = this.defines.concat(project.defines);
		this.parameters = this.parameters.concat(project.parameters);
		this.libraries = this.libraries.concat(project.libraries);
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
	 * Add all assets matching the match glob relative to the directory containing the current project.js.
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

	addParameter(parameter) {
		this.parameters.push(parameter);
	}

	addLibrary(library) {
		this.addDefine(library);
		let self = this;
		function findLibraryDirectory(name) {
			if (path.isAbsolute(name)) {
				return { libpath: name, libroot: name };
			}
			// Check relative path
			if (fs.existsSync(path.resolve(name))) {
				return { libpath: name, libroot: name };
			}
			console.error('Error: Library ' + name + ' not found.');
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
		}
	}
}

async function loadProject(from, projectfile) {
	return new Promise((resolve, reject) => {
		fs.readFile(path.join(from, projectfile), 'utf8', (err, data) => {
			if (err) {
				throw new Error('Error reading ' + projectfile + ' from ' + from + '.');
			}
			let resolved = false;
			let resolver = (project) => {
				resolved = true;
				resolve(project);
			};
			process.on('exit', (code) => {
				if (!resolved) {
					console.error('Error: project.js did not call resolve, no project created.');
				}
			});
			Project.scriptdir = from;
			try {
				let AsyncFunction = Object.getPrototypeOf(async () => { }).constructor;
				new AsyncFunction('Project', 'platform', 'require', 'process', 'resolve', 'reject', data)(Project, platform, require, process, resolver, reject);
			}
			catch (error) {
				reject(error);
			}
		});
	});
}

class AssetConverter {
	constructor(exporter, options, assetMatchers) {
		this.exporter = exporter;
		this.options = options;
		this.assetMatchers = assetMatchers;
	}

	close() {}

	static replacePattern(pattern, value, fileinfo, options, from) {
		let basePath = options.nameBaseDir ? path.join(from, options.nameBaseDir) : from;
		let dirValue = path.relative(basePath, fileinfo.dir);
		if (basePath.length > 0 && basePath[basePath.length - 1] === path.sep
			&& dirValue.length > 0 && dirValue[dirValue.length - 1] !== path.sep) {
			dirValue += path.sep;
		}
		if (options.namePathSeparator) {
			dirValue = dirValue.split(path.sep).join(options.namePathSeparator);
		}
		const dirRegex = dirValue === ''
			? /{dir}\//g
			: /{dir}/g;
		return pattern.replace(/{name}/g, value).replace(/{ext}/g, fileinfo.ext).replace(dirRegex, dirValue);
	}

	static createExportInfo(fileinfo, keepextension, options, from) {
		let nameValue = fileinfo.name;
		let destination = fileinfo.name;
		if ((keepextension || options.noprocessing) && (!options.destination || options.destination.indexOf('{ext}') < 0)) {
			destination += fileinfo.ext;
		}
		if (options.destination) {
			destination = AssetConverter.replacePattern(options.destination, destination, fileinfo, options, from);
		}
		if (keepextension && (!options.name || options.name.indexOf('{ext}') < 0)) {
			nameValue += fileinfo.ext;
		}
		if (options.name) {
			nameValue = AssetConverter.replacePattern(options.name, nameValue, fileinfo, options, from);
		}
		return { name: nameValue, destination: destination };
	}

	watch(match, temp, options) {
		return new Promise((resolve, reject) => {
			let basedir = match.substring(0, match.lastIndexOf("/") + 1);
			let pattern = match;
			if (path.isAbsolute(pattern)) {
				let _pattern = pattern;
				_pattern = path.relative(basedir, _pattern);
				pattern = _pattern;
			}
			let files = searchFiles(basedir, pattern);
			const self = this;

			async function convertAssets() {
				let parsedFiles = [];
				async function convertAsset(file, index) {
					let fileinfo = path.parse(file);
					console.log('Exporting asset ' + (index + 1) + ' of ' + files.length + ' (' + fileinfo.base + ').');
					const ext = fileinfo.ext.toLowerCase();
					switch (ext) {
						case '.png':
						case '.jpg':
						case '.hdr': {
							let exportInfo = AssetConverter.createExportInfo(fileinfo, false, options, self.exporter.options.from);
							let images;
							if (options.noprocessing) {
								images = await self.exporter.copyBlob(file, exportInfo.destination, options);
							}
							else {
								images = await self.exporter.copyImage(file, exportInfo.destination, options);
							}
							parsedFiles.push({ name: exportInfo.name, from: file, type: 'image', files: images, original_width: options.original_width, original_height: options.original_height, readable: options.readable, embed: options.embed });
							break;
						}
						case '.ogg':
						case '.mp3':
						case '.flac':
						case '.wav': {
							let exportInfo = AssetConverter.createExportInfo(fileinfo, false, options, self.exporter.options.from);
							let sounds;
							if (options.noprocessing) {
								sounds = await self.exporter.copyBlob(file, exportInfo.destination, options);
							}
							else {
								sounds = await self.exporter.copySound(file, exportInfo.destination, options);
							}
							if (sounds.length === 0) {
								throw 'Audio file ' + file + ' could not be exported, you have to specify a path to ffmpeg.';
							}
							parsedFiles.push({ name: exportInfo.name, from: file, type: 'sound', files: sounds, original_width: undefined, original_height: undefined, readable: undefined, embed: options.embed });
							break;
						}
						case '.ttf':
						case '.ttc':
						case '.otf': {
							let exportInfo = AssetConverter.createExportInfo(fileinfo, false, options, self.exporter.options.from);
							let fonts;
							if (options.noprocessing) {
								fonts = await self.exporter.copyBlob(file, exportInfo.destination, options);
							}
							else {
								fonts = await self.exporter.copyFont(file, exportInfo.destination + fileinfo.ext, options);
							}
							parsedFiles.push({ name: exportInfo.name, from: file, type: 'font', files: fonts, original_width: undefined, original_height: undefined, readable: undefined, embed: options.embed });
							break;
						}
						case '.mp4':
						case '.webm':
						case '.mov':
						case '.wmv':
						case '.avi': {
							let exportInfo = AssetConverter.createExportInfo(fileinfo, false, options, self.exporter.options.from);
							let videos;
							if (options.noprocessing) {
								videos = await self.exporter.copyBlob(file, exportInfo.destination, options);
							}
							else {
								videos = await self.exporter.copyVideo(file, exportInfo.destination, options);
							}
							if (videos.length === 0) {
								console.error('Video file ' + file + ' could not be exported, you have to specify a path to ffmpeg.');
							}
							parsedFiles.push({ name: exportInfo.name, from: file, type: 'video', files: videos, original_width: undefined, original_height: undefined, readable: undefined, embed: options.embed });
							break;
						}
						default: {
							let exportInfo = AssetConverter.createExportInfo(fileinfo, true, options, self.exporter.options.from);
							let blobs = await self.exporter.copyBlob(file, exportInfo.destination, options);
							parsedFiles.push({ name: exportInfo.name, from: file, type: 'blob', files: blobs, original_width: undefined, original_height: undefined, readable: undefined, embed: options.embed });
							break;
						}
					}
				}
				let index = 0;
				for (let file of files) {
					await convertAsset(file, index);
					index += 1;
				}
				fs.ensureDirSync(temp);
				resolve(parsedFiles);
			};
			convertAssets();
		});
	}

	async run(temp) {
		let files = [];
		for (let matcher of this.assetMatchers) {
			files = files.concat(await this.watch(matcher.match, temp, matcher.options));
		}
		return files;
	}
}

class CompiledShader {
	constructor() {
		this.files = [];
		this.embed = false;
	}
}

class ShaderCompiler {
	constructor(exporter, compiler, to, temp, builddir, options, shaderMatchers) {
		this.exporter = exporter;
		this.compiler = compiler;
		this.type = ShaderCompiler.findType(options);
		this.options = options;
		this.to = to;
		this.temp = temp;
		this.builddir = builddir;
		this.shaderMatchers = shaderMatchers;
	}

	close() {}

	static findType(options) {
		if (options.graphics === 'default') {
			if (process.platform === 'win32') {
				return 'd3d11';
			}
			else if (process.platform === 'darwin') {
				return 'metal';
			}
			else {
				return options.shaderversion == 300 ? 'essl' : 'glsl';
			}
		}
		else if (options.graphics === 'vulkan') {
			return 'spirv';
		}
		else if (options.graphics === 'metal') {
			return 'metal';
		}
		else if (options.graphics === 'opengl') {
			return options.shaderversion == 300 ? 'essl' : 'glsl';
		}
		else if (options.graphics === 'direct3d11' || options.graphics === 'direct3d12') {
			return 'd3d11';
		}
		else {
			throw new Error('Unsupported shader language.');
		}
	}

	watch(match, options, recompileAll) {
		return new Promise((resolve, reject) => {
			let basedir = match.substring(0, match.lastIndexOf("/") + 1);
			let pattern = match;
			if (path.isAbsolute(pattern)) {
				let _pattern = pattern;
				_pattern = path.relative(basedir, _pattern);
				pattern = _pattern;
			}
			let shaders = searchFiles(basedir, pattern);
			const self = this;
			async function compileShaders() {
				let compiledShaders = [];
				async function compile(shader, index) {
					let parsed = path.parse(shader);
					console.log('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ').');
					let compiledShader = null;
					try {
						compiledShader = await self.compileShader(shader, options, recompileAll);
					}
					catch (error) {
						console.error('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ') failed:');
						console.error(error);
						return Promise.reject(error);
					}
					if (compiledShader === null) {
						compiledShader = new CompiledShader();
						compiledShader.embed = options.embed;
						// mark variables as invalid, so they are loaded from previous compilation
						compiledShader.files = null;
					}
					if (compiledShader.files != null && compiledShader.files.length === 0) {
						compiledShader.files.push('data/' + parsed.name + '.' + self.type);
					}
					compiledShader.name = AssetConverter.createExportInfo(parsed, false, options, self.exporter.options.from).name;
					compiledShaders.push(compiledShader);
					++index;
					return Promise.resolve();
				}
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
				resolve(compiledShaders);
				return;
			}
			compileShaders();
		});
	}

	async run(recompileAll) {
		let shaders = [];
		for (let matcher of this.shaderMatchers) {
			shaders = shaders.concat(await this.watch(matcher.match, matcher.options, recompileAll));
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
							fs.copyFileSync(from, to);
						}
						let compiledShader = new CompiledShader();
						compiledShader.embed = options.embed;
						resolve(compiledShader);
						return;
					}
					fs.stat(this.compiler, (compErr, compStats) => {
						if (!recompile && (fromErr || (!toErr && toStats.mtime.getTime() > fromStats.mtime.getTime() && toStats.mtime.getTime() > compStats.mtime.getTime()))) {
							if (fromErr)
								console.error('Shader compiler error: ' + fromErr);
							resolve(null);
						}
						else {
							let parameters = [this.type === 'hlsl' ? 'd3d9' : this.type, from, temp, this.temp, 'krom'];
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
								console.log(data.toString());
							});
							let errorLine = '';
							let newErrorLine = true;
							let errorData = false;
							let compiledShader = new CompiledShader();
							compiledShader.embed = options.embed;
							child.stderr.on('data', (data) => {
								let str = data.toString();
								for (let char of str) {
									if (char === '\n') {
										if (errorData) {}
										else {
											console.error(errorLine.trim());
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
									if (errorData) {}
									else {
										console.error(errorLine.trim());
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

function convertImage(from, temp, to, root, exe, params) {
	return new Promise((resolve, reject) => {
		let process = child_process.spawn(path.join(root, 'Kinc', 'Tools', sysdir(), exe), params);
		process.stdout.on('data', (data) => {});
		process.stderr.on('data', (data) => {});
		process.on('close', (code) => {
			if (code !== 0) {
				console.error('kraffiti process exited with code ' + code + ' when trying to convert ' + path.parse(from).name);
				resolve();
				return;
			}
			fs.renameSync(temp, to);
			resolve();
		});
	});
}

async function exportImage(root, from, to) {
	to += '.k';
	let temp = to + '.temp';
	let outputformat = 'k';
	if (fs.existsSync(to) && fs.statSync(to).mtime.getTime() > fs.statSync(from.toString()).mtime.getTime()) {
		return outputformat;
	}
	fs.ensureDirSync(path.dirname(to));
	const exe = 'kraffiti' + sys();
	let params = ['from=' + from, 'to=' + temp, 'format=lz4'];
	params.push('filter=nearest');
	await convertImage(from, temp, to, root, exe, params);
	return outputformat;
}

function convertEncoder(inFilename, outFilename, encoder, args = null) {
	return new Promise((resolve, reject) => {
		if (fs.existsSync(outFilename.toString()) && fs.statSync(outFilename.toString()).mtime.getTime() > fs.statSync(inFilename.toString()).mtime.getTime()) {
			resolve(true);
			return;
		}
		if (!encoder) {
			resolve(false);
			return;
		}
		let dirend = Math.max(encoder.lastIndexOf('/'), encoder.lastIndexOf('\\'));
		let firstspace = encoder.indexOf(' ', dirend);
		let exe = encoder.substr(0, firstspace);
		let parts = encoder.substr(firstspace + 1).split(' ');
		let options = [];
		for (let i = 0; i < parts.length; ++i) {
			let foundarg = false;
			if (args !== null) {
				for (let arg in args) {
					if (parts[i] === '{' + arg + '}') {
						options.push(args[arg]);
						foundarg = true;
						break;
					}
				}
			}
			if (foundarg)
				continue;
			if (parts[i] === '{in}')
				options.push(inFilename.toString());
			else if (parts[i] === '{out}')
				options.push(outFilename.toString());
			else
				options.push(parts[i]);
		}
		// About stdio ignore: https://stackoverflow.com/a/20792428
		let process = child_process.spawn(exe, options, { stdio: 'ignore' });
		process.on('close', (code) => {
			resolve(code === 0);
		});
	});
}

class ArmorCoreExporter {
	constructor(options) {
		this.options = options;
		this.sources = [];
		this.libraries = [];
		this.addSourceDirectory(path.join(options.root, 'Sources'));
		this.addSourceDirectory(path.join(options.root, 'Sources/iron'));
		this.addSourceDirectory(path.join(options.root, 'Sources/zui'));
		this.projectFiles = !options.noproject;
		this.parameters = [];
	}

	tsOptions(name, defines) {
		let graphics = this.options.graphics;
		if (graphics === 'default') {
			if (process.platform === 'win32') {
				graphics = 'direct3d11';
			}
			else if (process.platform === 'darwin') {
				graphics = 'metal';
			}
			else {
				graphics = 'opengl';
			}
		}
		defines.push('krom_' + graphics);
		if (process.argv.indexOf('android') >= 0) {
			defines.push('krom_android');
		}
		else if (process.argv.indexOf('ios') >= 0) {
			defines.push('krom_ios');
		}
		else if (process.platform === 'win32') {
			defines.push('krom_windows');
		}
		else if (process.platform === 'linux') {
			defines.push('krom_linux');
		}
		else if (process.platform === 'darwin') {
			defines.push('krom_darwin');
		}
		return {
			from: this.options.from.toString(),
			to: path.join(this.sysdir(), 'krom.js.temp'),
			realto: path.join(this.sysdir(), 'krom.js'),
			sources: this.sources,
			libraries: this.libraries,
			defines: defines,
			parameters: this.parameters,
			tsDirectory: this.options.haxe,
			system: this.sysdir(),
			language: 'js',
			width: this.width,
			height: this.height,
			name: name,
		};
	}

	async export(name, tsOptions) {
		fs.ensureDirSync(path.join(this.options.to, this.sysdir()));
	}

	async copySound(from, to, options) {
		fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
		if (options.quality < 1) {
			let ogg = await convertEncoder(from, path.join(this.options.to, this.sysdir(), to + '.ogg'), this.options.ogg);
			return [to + '.ogg'];
		}
		else {
			fs.copyFileSync(from.toString(), path.join(this.options.to, this.sysdir(), to + '.wav'));
			return [to + '.wav'];
		}
	}

	async copyImage(from, to, options) {
		let format = await exportImage(this.options.root, from, path.join(this.options.to, this.sysdir(), to));
		return [to + '.' + format];
	}

	async copyBlob(from, to) {
		fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
		fs.copyFileSync(from.toString(), path.join(this.options.to, this.sysdir(), to));
		return [to];
	}

	async copyVideo(from, to) {
		fs.ensureDirSync(path.join(this.options.to, this.sysdir(), path.dirname(to)));
		let webm = await convertEncoder(from, path.join(this.options.to, this.sysdir(), to + '.webm'), this.options.webm);
		let files = [];
		if (webm)
			files.push(to + '.webm');
		return files;
	}

	sysdir() {
		return this.systemDirectory;
	}

	setName(name) {
		this.name = name;
		this.safename = name.replace(/ /g, '-');
	}

	setSystemDirectory(systemDirectory) {
		this.systemDirectory = systemDirectory;
	}

	addSourceDirectory(path) {
		this.sources.push(path);
	}

	addLibrary(library) {
		this.libraries.push(library);
	}

	async copyFont(from, to, options) {
		return await this.copyBlob(from, to, options);
	}
}

function writeTSProject(projectdir, projectFiles, options) {
	let tsdata = {
		include: [],
		compilerOptions: {
			lib: ["es2015"],
			target: "es2015",
			removeComments: true,
			alwaysStrict: true,
			noImplicitAny: true,
			noImplicitReturns: true,
			noUncheckedIndexedAccess: true,
			strictPropertyInitialization: false,
			incremental: true,
			tsBuildInfoFile: "./krom.tsbuildinfo"
			// noFallthroughCasesInSwitch
			// noUnusedLocals
			// noUnusedParameters
			// strictNullChecks
			// allowUnreachableCode
		}
	};

	let main_ts = null;

	for (let i = 0; i < options.sources.length; ++i) {
		if (fs.existsSync(options.sources[i])) {
			let files = fs.readdirSync(options.sources[i]);
			for (let file of files) {
				if (file.endsWith(".ts")) {
					// Prevent duplicates, keep the newly added file
					for (let included of tsdata.include){
						if (path.basename(included) == file) {
							tsdata.include.splice(tsdata.include.indexOf(included), 1);
							break;
						}
					}
					tsdata.include.push(options.sources[i] + "/" + file);

					if (file == "main.ts") main_ts = options.sources[i] + "/" + file;
				}
			}
		}
	}

	// Include main.ts last
	if (main_ts != null) {
		tsdata.include.splice(tsdata.include.indexOf(main_ts), 1);
		tsdata.include.push(main_ts);
	}

	fs.writeFileSync(path.join(projectdir, 'tsconfig.json'), JSON.stringify(tsdata, null, 4));

	// Hermes compiler
	if (globalThis.flags.with_hermes) {
		let shermes = __dirname + '/hermes/linux/shermes';
		let include = __dirname + '/hermes/include';

		// '-parse-ts' '-emit-c'
		let result = child_process.spawnSync(shermes, ['-c', '-typed', '-Wc,-I' + include, '-o', 'build/main.o', 'armorcore/Sources/hermes/Krom.ts', 'Sources/main.ts']);
		if (result.status !== 0) {
			console.log(result.stderr.toString());
			process.exit();
		}
		child_process.spawnSync('ar', ['-rcs', 'build/libmain.a', 'build/main.o']);
		child_process.spawnSync('objcopy', ['--redefine-sym', 'main=sh_main', 'build/libmain.a', 'build/libmain.a']);
	}
	// TS compiler
	else {
		let tsc = path.resolve(__dirname + '/Tools/tsc/tsc.js');
		globalThis.require = require;
		globalThis.module = {};
		globalThis.__filename = tsc;

		let containsDefine = (define) => {
			let b = false;
			for (let s of options.defines) if (define.includes(s)) { b = true; break; };
			if (define.includes("!")) b = !b;
			return b;
		}

		globalThis.ts_preprocessor = (file) => {
			let stack = [];
			let found = [];
			let lines = file.split("\n");
			for (let i = 0; i < lines.length; ++i) {
				let line = lines[i].trimStart();
				if (line.startsWith("///if")) {
					let define = line.substr(6);
					stack.push(containsDefine(define));
					found.push(stack[stack.length - 1]);
				}
				else if (line.startsWith("///elseif")) {
					let define = line.substr(10);
					if (!found[found.length - 1] && containsDefine(define)) {
						stack[stack.length - 1] = true;
						found[found.length - 1] = true;
					}
					else {
						stack[stack.length - 1] = false;
					}
				}
				else if (line.startsWith("///else")) {
					stack[stack.length - 1] = !found[found.length - 1];
				}
				else if (line.startsWith("///end")) {
					stack.pop();
					found.pop();
				}
				else if (stack.length > 0) {
					let comment = false;
					for (b of stack) {
						if (!b) {
							comment = true;
							break;
						}
					}
					if (comment) {
						lines[i] = "///" + lines[i];
					}
				}
			}
			return lines.join("\n");
		};

		let _cwd = process.cwd();
		process.chdir(projectdir + '/krom')
		let _argv = process.argv;
		process.argv = [];
		process.argv.push('tsc.js');
		process.argv.push('.');
		process.argv.push('--outFile');
		process.argv.push('krom.js');
		(1, eval)(fs.readFileSync(tsc) + '');
		process.argv = _argv;
		process.chdir(_cwd);
	}
}

let options = [
	{
		full: 'from',
		value: true,
		description: 'Location of your project',
		default: '.'
	},
	{
		full: 'to',
		value: true,
		description: 'Build location',
		default: 'build'
	},
	{
		full: 'graphics',
		short: 'g',
		description: 'Graphics api to use. Possible parameters are direct3d9, direct3d11, direct3d12, metal, vulkan and opengl.',
		value: true,
		default: 'default'
	},
	{
		full: 'ffmpeg',
		description: 'Location of ffmpeg executable',
		value: true,
		default: ''
	},
	{
		full: 'shaderversion',
		description: 'Set target shader version manually.',
		value: true,
		default: null
	},
	{
		full: 'snapshot',
		description: 'Generate v8 snapshot file.',
		value: false,
		default: null
	},
];

let parsedOptions = {};

function printHelp() {
	console.log('make options:\n');
	for (let option of options) {
		if (option.hidden)
			continue;
		if (option.short)
			console.log('-' + option.short + ' ' + '--' + option.full);
		else
			console.log('--' + option.full);
		console.log(option.description);
		console.log();
	}
}

for (let option of options) {
	if (option.value) {
		parsedOptions[option.full] = option.default;
	}
	else {
		parsedOptions[option.full] = false;
	}
}

let args = process.argv;
for (let i = 2; i < args.length; ++i) {
	let arg = args[i];
	if (arg[0] === '-') {
		if (arg[1] === '-') {
			if (arg.substr(2) === 'help') {
				printHelp();
				process.exit(0);
			}
			for (let option of options) {
				if (arg.substr(2) === option.full) {
					if (option.value) {
						++i;
						parsedOptions[option.full] = args[i];
					}
					else {
						parsedOptions[option.full] = true;
					}
				}
			}
		}
		else {
			if (arg[1] === 'h') {
				printHelp();
				process.exit(0);
			}
			for (let option of options) {
				if (option.short && arg[1] === option.short) {
					if (option.value) {
						++i;
						parsedOptions[option.full] = args[i];
					}
					else {
						parsedOptions[option.full] = true;
					}
				}
			}
		}
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

async function exportProjectFiles(name, resourceDir, options, exporter, kore, korehl, libraries, defines, id) {
	if (options.haxe !== '') {
		let tsOptions = exporter.tsOptions(name, defines);
		tsOptions.safeName = safeName(tsOptions.name);
		writeTSProject(options.to, !options.noproject, tsOptions);
		await exporter.export(name, tsOptions);
	}

	console.log('Done.');
	return name;
}

async function exportArmorCoreProject(options) {
	console.log('Creating ArmorCore project files.');
	let project = null;
	let foundProjectFile = false;
	// get the project.js and load the config code,
	// then create the project config object, which contains stuff
	// like project name, assets paths, sources path, library path...
	if (fs.existsSync(path.join(options.from, 'project.js'))) {
		try {
			project = await loadProject(options.from, 'project.js');
		}
		catch (x) {
			console.error(x);
			throw 'Loading the project.js file failed.';
		}
		foundProjectFile = true;
	}
	if (!foundProjectFile) {
		throw 'No project.js found.';
	}
	let temp = path.join(options.to, 'temp');
	fs.ensureDirSync(temp);
	let exporter = null;
	let kore = false;
	let korehl = false;
	exporter = new ArmorCoreExporter(options);
	exporter.setSystemDirectory('krom');
	let buildDir = path.join(options.to, exporter.sysdir() + '-build');
	// Create the target build folder
	// e.g. 'build/krom'
	fs.ensureDirSync(path.join(options.to, exporter.sysdir()));
	exporter.setName(project.name);
	for (let source of project.sources) {
		exporter.addSourceDirectory(source);
	}
	for (let library of project.libraries) {
		exporter.addLibrary(library);
	}
	exporter.parameters = exporter.parameters.concat(project.parameters);
	project.scriptdir = options.root;

	let assetConverter = new AssetConverter(exporter, options, project.assetMatchers);
	let assets = await assetConverter.run(temp);
	let shaderDir = path.join(options.to, exporter.sysdir(), 'data');

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
	let shaderCompiler = new ShaderCompiler(exporter, options.krafix, shaderDir, temp, buildDir, options, project.shaderMatchers);
	try {
		exportedShaders = await shaderCompiler.run(recompileAllShaders);
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
	let embed_files = [];
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
		if (asset.embed) embed_files.push(file);
	}
	for (let shader of exportedShaders) {
		let oldShader = findShader(shader.name);
		let file = {
			name: fixName(shader.name),
			files: shader.files === null ? oldShader.files : shader.files,
			type: 'shader'
		};
		files.push(file);
		if (shader.embed) embed_files.push(file);
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
		fs.ensureDirSync(path.join(options.to, exporter.sysdir() + '-resources'));
		fs.writeFileSync(path.join(options.to, exporter.sysdir() + '-resources', 'files.json'), JSON.stringify({ files: files }, null, '\t'));
		if (embed_files.length > 0) {
			let embed_string = "";
			for (let file of embed_files) {
				embed_string += file.files[0] + '\n';
			}
			fs.ensureDirSync(path.join(options.to, exporter.sysdir(), 'data'));
			fs.writeFileSync(path.join(options.to, exporter.sysdir(), 'data', 'embed.txt'), embed_string);
		}
	}

	return await exportProjectFiles(project.name, path.join(options.to, exporter.sysdir() + '-resources'), options, exporter, kore, korehl, project.libraries, project.defines, project.id);
}

function isProject(directory, projectfile) {
	return fs.existsSync(path.join(directory, projectfile));
}

async function exportProject(options) {
	if (isProject(options.from, 'project.js')) {
		return await exportArmorCoreProject(options);
	}
	else {
		console.error('project.js file not found.');
		return 'Unknown';
	}
}

async function run(options) {
	let p = __dirname;
	if (fs.existsSync(p) && fs.statSync(p).isDirectory()) {
		options.root = p;
	}
	console.log('Using ArmorCore from ' + options.root + ".");
	let haxepath = path.join(options.root, 'Tools', 'haxe');
	if (fs.existsSync(haxepath) && fs.statSync(haxepath).isDirectory())
		options.haxe = haxepath;
	let krafixpath = path.join(options.root, 'Kinc', 'Tools', sysdir(), 'krafix' + sys());
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
		let oggpath = path.join(options.root, 'Tools', sysdir(), 'oggenc' + sys());
		if (fs.existsSync(oggpath))
			options.ogg = oggpath + ' {in} -o {out} --quiet';
	}
	if (!options.mp3) {
		let lamepath = path.join(options.root, 'Tools', sysdir(), 'lame' + sys());
		if (fs.existsSync(lamepath))
			options.mp3 = lamepath + ' {in} {out}';
	}
	let name = '';
	try {
		name = await exportProject(options);
	}
	catch (error) {
		console.log(error);
		process.exit(1);
	}
	return name;
}

async function make_run() {
	try {
		await run(parsedOptions, (name) => { });
	}
	catch (error) {
		console.log(error);
		process.exit(1);
	}
}
