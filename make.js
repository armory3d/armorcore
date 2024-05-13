const fs = require("fs");
const os = require("os");
const path = require("path");
const child_process = require("child_process");

function ensureDirSync(dir) {
	const parent = path.normalize(path.join(dir, '..'));
	if (!fs.existsSync(parent)) {
		ensureDirSync(parent);
	}
	if (!fs.existsSync(dir)) {
		fs.mkdirSync(dir);
	}
}
fs.ensureDirSync = ensureDirSync;

function exe_ext() {
	if (os.platform() === 'win32') {
		return '.exe';
	}
	else {
		return '';
	}
}

function sys_dir() {
	if (os.platform() === 'linux') {
		if (os.arch() === 'arm64') return 'linux_arm64';
		else if (os.arch() === 'x64') return 'linux_x64';
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
	return path.replaceAll("\\", '/');
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
		this.scriptdir = Project.scriptdir;
		this.assetMatchers = [];
		this.shaderMatchers = [];
	}

	addProject(projectDir) {
		let project = loadProject(projectDir, 'project.js');
		this.assetMatchers = this.assetMatchers.concat(project.assetMatchers);
		this.sources = this.sources.concat(project.sources);
		this.shaderMatchers = this.shaderMatchers.concat(project.shaderMatchers);
		this.defines = this.defines.concat(project.defines);
	}

	addAssets(match, options) {
		if (!options)
			options = {};
		if (!path.isAbsolute(match)) {
			let base = stringify(path.resolve(this.scriptdir));
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
			let base = stringify(path.resolve(this.scriptdir));
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
}

function loadProject(from, projectfile) {
	let data = fs.readFileSync(path.join(from, projectfile));
	Project.scriptdir = from;
	let fn = new Function('Project', 'platform', 'require', 'process', data);
	return fn(Project, platform, require, process);
}

class AssetConverter {
	constructor(exporter, options, assetMatchers) {
		this.exporter = exporter;
		this.options = options;
		this.assetMatchers = assetMatchers;
	}

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
		return pattern.replace(/{name}/g, value).replace(dirRegex, dirValue);
	}

	static createExportInfo(fileinfo, keepextension, options, from) {
		let nameValue = fileinfo.name;
		let destination = fileinfo.name;
		if (keepextension || options.noprocessing) {
			destination += fileinfo.ext;
		}
		if (options.destination) {
			destination = AssetConverter.replacePattern(options.destination, destination, fileinfo, options, from);
		}
		if (keepextension) {
			nameValue += fileinfo.ext;
		}
		if (options.name) {
			nameValue = AssetConverter.replacePattern(options.name, nameValue, fileinfo, options, from);
		}
		return { name: nameValue, destination: destination };
	}

	watch(match, temp, options) {
		let basedir = match.substring(0, match.lastIndexOf("/") + 1);
		let pattern = match;
		if (path.isAbsolute(pattern)) {
			let _pattern = pattern;
			_pattern = path.relative(basedir, _pattern);
			pattern = _pattern;
		}
		let files = searchFiles(basedir, pattern);
		const self = this;

		let parsedFiles = [];

		let index = 0;
		for (let file of files) {
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
						images = self.exporter.copyBlob(file, exportInfo.destination, options);
					}
					else {
						images = self.exporter.copyImage(file, exportInfo.destination, options);
					}
					parsedFiles.push({ name: exportInfo.name, from: file, type: 'image', files: images, original_width: options.original_width, original_height: options.original_height, readable: options.readable, embed: options.embed });
					break;
				}
				default: {
					let exportInfo = AssetConverter.createExportInfo(fileinfo, true, options, self.exporter.options.from);
					let blobs = self.exporter.copyBlob(file, exportInfo.destination, options);
					parsedFiles.push({ name: exportInfo.name, from: file, type: 'blob', files: blobs, original_width: undefined, original_height: undefined, readable: undefined, embed: options.embed });
					break;
				}
			}

			index += 1;
		}
		return parsedFiles;
	}

	run(temp) {
		let files = [];
		for (let matcher of this.assetMatchers) {
			files = files.concat(this.watch(matcher.match, temp, matcher.options));
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
		this.shaderMatchers = shaderMatchers;
	}

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
	}

	watch(match, options, recompileAll) {
		let basedir = match.substring(0, match.lastIndexOf("/") + 1);
		let pattern = match;
		if (path.isAbsolute(pattern)) {
			let _pattern = pattern;
			_pattern = path.relative(basedir, _pattern);
			pattern = _pattern;
		}
		let shaders = searchFiles(basedir, pattern);
		const self = this;

		let compiledShaders = [];

		let index = 0;
		for (let shader of shaders) {
			let parsed = path.parse(shader);
			console.log('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ').');
			let compiledShader = null;
			try {
				compiledShader = self.compileShader(shader, options, recompileAll);
			}
			catch (error) {
				console.error('Compiling shader ' + (index + 1) + ' of ' + shaders.length + ' (' + parsed.base + ') failed:');
				console.error(error);
			}
			if (compiledShader === null) {
				compiledShader = new CompiledShader();
				compiledShader.embed = options.embed;
				compiledShader.files = null;
			}
			if (compiledShader.files != null && compiledShader.files.length === 0) {
				compiledShader.files.push('data/' + parsed.name + '.' + self.type);
			}
			compiledShader.name = AssetConverter.createExportInfo(parsed, false, options, self.exporter.options.from).name;
			compiledShaders.push(compiledShader);
			++index;
		}

		return compiledShaders;
	}

	run() {
		let shaders = [];
		for (let matcher of this.shaderMatchers) {
			shaders = shaders.concat(this.watch(matcher.match, matcher.options));
		}
		return shaders;
	}

	compileShader(file, options) {
		let fileinfo = path.parse(file);
		let from = file;
		let to = path.join(this.to, fileinfo.name + '.' + this.type);

		let fromStats;
		if (fs.existsSync(from)) fromStats = fs.statSync(from);
		let toStats;
		if (fs.existsSync(to)) toStats = fs.statSync(to);

		if (options.noprocessing) {
			if (!toStats || toStats.mtime.getTime() < fromStats.mtime.getTime()) {
				fs.copyFileSync(from, to);
			}
			let compiledShader = new CompiledShader();
			compiledShader.embed = options.embed;
			return compiledShader;
		}

		let compStats = fs.statSync(this.compiler);

		if (!fromStats || (toStats && toStats.mtime.getTime() > fromStats.mtime.getTime() && toStats.mtime.getTime() > compStats.mtime.getTime())) {
			return null;
		}
		else {
			let parameters = [this.type, from, to, this.temp, 'krom'];
			fs.ensureDirSync(this.temp);
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

			let child = child_process.spawnSync(this.compiler, parameters);
			if (child.status !== 0) {
				console.error('Shader compiler error.')
			}

			let compiledShader = new CompiledShader();
			compiledShader.embed = options.embed;
			return compiledShader;
		}
	}
}

function convertImage(from, temp, to, root, exe, params) {
	child_process.spawnSync(path.join(root, 'Kinc', 'Tools', sys_dir(), exe), params)
	fs.renameSync(temp, to);
}

function exportImage(root, from, to) {
	to += '.k';
	let temp = to + '.temp';
	let outputformat = 'k';
	if (fs.existsSync(to) && fs.statSync(to).mtime.getTime() > fs.statSync(from.toString()).mtime.getTime()) {
		return outputformat;
	}
	fs.ensureDirSync(path.dirname(to));
	const exe = 'kraffiti' + exe_ext();
	let params = ['from=' + from, 'to=' + temp, 'format=lz4'];
	params.push('filter=nearest');
	convertImage(from, temp, to, root, exe, params);
	return outputformat;
}

class ArmorCoreExporter {
	constructor(options) {
		this.options = options;
		this.sources = [];
		this.addSourceDirectory(path.join(__dirname, 'Sources'));
		if (globalThis.flags.with_iron) {
			this.addSourceDirectory(path.join(__dirname, 'Sources/iron'));
		}
		if (globalThis.flags.with_zui) {
			this.addSourceDirectory(path.join(__dirname, 'Sources/zui'));
		}
		this.projectFiles = !options.noproject;
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
			sources: this.sources,
			defines: defines
		};
	}

	export(name, tsOptions) {
		fs.ensureDirSync(path.join(this.options.to, 'krom'));
	}

	copyImage(from, to, options) {
		let format = exportImage(__dirname, from, path.join(this.options.to, 'krom', to));
		return [to + '.' + format];
	}

	copyBlob(from, to) {
		fs.ensureDirSync(path.join(this.options.to, 'krom', path.dirname(to)));
		fs.copyFileSync(from.toString(), path.join(this.options.to, 'krom', to));
		return [to];
	}

	addSourceDirectory(path) {
		this.sources.push(path);
	}
}

function ts_preprocessor(file, file_path) {
	let contains_define = (define) => {
		let b = false;
		for (let s of globalThis.options.defines) {
			if (define.includes(s)) {
				b = true;
				break;
			};
		}
		if (define.includes("!")) {
			b = !b;
		}
		return b;
	}

	let stack = [];
	let found = [];
	let lines = file.split("\n");
	for (let i = 0; i < lines.length; ++i) {
		let line = lines[i].trimStart();
		if (line.startsWith("///if")) {
			let define = line.substr(6);
			stack.push(contains_define(define));
			found.push(stack[stack.length - 1]);
		}
		else if (line.startsWith("///elseif")) {
			let define = line.substr(10);
			if (!found[found.length - 1] && contains_define(define)) {
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
		if (lines[i].indexOf("__ID__") > -1 && !lines[i].startsWith("declare")) {
			// #define ID__(x, y) x ":" #y
			// #define ID_(x, y) ID__(x, y)
			// #define ID ID_(__FILE__, __LINE__)
			lines[i] = lines[i].replace("__ID__", "\"" + path.basename(file_path) + ":" + i + "\"");
		}
	}
	return lines.join("\n");
}

function writeTSProject(projectdir, projectFiles, options) {
	let tsdata = {
		include: []
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

	// MiniTS compiler
	globalThis.options = options;
	globalThis.require = require;
	let source = '';
	let file_paths = JSON.parse(fs.readFileSync('build/tsconfig.json')).include;
	for (let file_path of file_paths) {
		let file = fs.readFileSync(file_path) + '';
		file = ts_preprocessor(file, file_path);
		source += file;
	}
	globalThis.flags.minits_source = source;
	globalThis.flags.minits_output = process.cwd() + "/build/krom.c";
	let minits = __dirname + '/Tools/minits/minits.js';
	(1, eval)(fs.readFileSync(minits) + '');
}

function exportProjectFiles(name, resourceDir, options, exporter, defines, id) {
	let tsOptions = exporter.tsOptions(name, defines);
	writeTSProject(options.to, !options.noproject, tsOptions);
	exporter.export(name, tsOptions);

	console.log('Done.');
	return name;
}

function exportArmorCoreProject(options) {
	console.log('Creating ArmorCore project files.');
	let project = null;
	if (fs.existsSync(path.join(options.from, 'project.js'))) {
		try {
			project = loadProject(options.from, 'project.js');
		}
		catch (x) {
			console.error(x);
		}
	}

	if (project == null) {
		fs.writeFileSync('build/krom.c', 'int kickstart(int argc, char **argv) { return 0; }\n');
		return;
	}

	let temp = path.join(options.to, 'temp');

	let exporter = new ArmorCoreExporter(options);
	let buildDir = path.join(options.to, 'krom-build');
	// Create the target build folder
	// e.g. 'build/krom'
	fs.ensureDirSync(path.join(options.to, 'krom'));

	for (let source of project.sources) {
		exporter.addSourceDirectory(source);
	}
	project.scriptdir = __dirname;

	let assetConverter = new AssetConverter(exporter, options, project.assetMatchers);
	let assets = assetConverter.run(temp);
	let shaderDir = path.join(options.to, 'krom', 'data');

	fs.ensureDirSync(shaderDir);

	let exportedShaders = [];
	let krafix = path.join(__dirname, 'Kinc', 'Tools', sys_dir(), 'krafix' + exe_ext())
	let shaderCompiler = new ShaderCompiler(exporter, krafix, shaderDir, temp, buildDir, options, project.shaderMatchers);
	try {
		exportedShaders = shaderCompiler.run();
	}
	catch (err) {
		return Promise.reject(err);
	}

	let embed_files = [];
	for (let asset of assets) {
		if (asset.embed) embed_files.push(file);
	}
	for (let shader of exportedShaders) {
		if (shader.embed) embed_files.push(shader);
	}

	if (embed_files.length > 0) {
		let embed_string = "";
		for (let file of embed_files) {
			embed_string += file.files[0] + '\n';
		}
		fs.ensureDirSync(path.join(options.to, 'krom', 'data'));
		fs.writeFileSync(path.join(options.to, 'krom', 'data', 'embed.txt'), embed_string);
	}

	exportProjectFiles(project.name, path.join(options.to, 'krom-resources'), options, exporter, project.defines, project.id);
}

let options = [
	{
		full: 'from',
		description: 'Location of your project',
		default: '.'
	},
	{
		full: 'to',
		description: 'Build location',
		default: 'build'
	},
	{
		full: 'graphics',
		short: 'g',
		description: 'Graphics api to use. Possible parameters are direct3d11, direct3d12, metal, vulkan and opengl.',
		default: 'default'
	},
	{
		full: 'shaderversion',
		description: 'Set target shader version manually.',
		default: null
	},
];

let parsedOptions = {};

for (let option of options) {
	parsedOptions[option.full] = option.default;
}

let args = process.argv;
for (let i = 2; i < args.length; ++i) {
	let arg = args[i];
	if (arg[0] === '-') {
		if (arg[1] === '-') {
			for (let option of options) {
				if (arg.substr(2) === option.full) {
					++i;
					parsedOptions[option.full] = args[i];
				}
			}
		}
		else {
			for (let option of options) {
				if (option.short && arg[1] === option.short) {
					++i;
					parsedOptions[option.full] = args[i];
				}
			}
		}
	}
}

console.log('Using ArmorCore from ' + __dirname + ".");
try {
	exportArmorCoreProject(parsedOptions);
}
catch (error) {
	console.log(error);
	process.exit(1);
}
