// Mini-kmake based on https://github.com/Kode/kmake by RobDangerous

import * as std from 'std';
import * as os from 'os';

function getEmbeddedBinaryData() {

}

function getEmbeddedData() {

}

function fs_exists(p) {
	let f = std.open(p, "rb");
	let exists = f != null;
	if (f != null) f.close();
	return exists;
}

function fs_mkdir(p) {
	os.mkdir(p);
}

function array_remove(arr, e) {
	let i = arr.indexOf(e);
	if (i != -1) {
		arr.splice(i, 1);
	}
}

function fs_readdir(p) {
	let dirs = os.readdir(p)[0];
	array_remove(dirs, ".");
	array_remove(dirs, "..");
	return dirs;
}

function fs_copyfile(from, to) {
	let f = std.open(from, "rb");
	f.seek(0, std.SEEK_END);
    let size = f.tell();
	let u8 = new Uint8Array(size);
    f.seek(0, std.SEEK_SET);
	f.read(u8.buffer, 0, size);
    f.close();

	f = std.open(to, "w");
	f.write(u8.buffer, 0, size);
	f.close();
}

function fs_rename(from, to) {
	os.rename(from, to);
}

function fs_isdir(p) {
	return (os.stat(p)[0].mode & os.S_IFMT) == os.S_IFDIR;
}

function fs_mtime(p) {
	return os.stat(p)[0].mtime;
}

function fs_readfile(p) {
	return std.loadFile(p);
}

function fs_writefile(p, data) {
	let u8 = new Uint8Array(data.length);
	for (let i = 0; i < data.length; ++i) {
		u8[i] = data.charCodeAt(i);
	}
	let f = std.open(p, "w");
	f.write(u8.buffer, 0, u8.length);
	f.close();
}

function parent_dir(dir) {
	return dir.substring(0, dir.lastIndexOf("/"));
}

function fs_ensuredir(dir) {
	if (dir != "" && !fs_exists(dir)) {
		fs_ensuredir(parent_dir(dir));
		fs_mkdir(dir);
	}
}

function fs_copydir(from, to) {
	fs_ensuredir(to);
	const files = fs_readdir(from);
	for (const file of files) {
		if (fs_isdir(path_join(from, file))) {
			fs_copydir(path_join(from, file), path_join(to, file));
		}
		else {
			fs_copyfile(path_join(from, file), path_join(to, file));
		}
	}
}

function os_exec(exe, params = [], ops = {}) {
	params.unshift(exe);
	// let pipe = os.pipe();
	// ops.stdout = pipe[1];
	// ops.block = false;
	let status = os.exec(params, ops);
	// os.close(pipe[1]);

	// let file = std.fdopen(pipe[0], 'r');
	// let s = file.readAsString().trim();
	// file.close();
	// os.waitpid(pid, 0);

	// let status = os.waitpid(pid, 0)[0];
	return { status: status };
}

function os_platform() {
	return os.platform;
}

function os_cwd() {
	return os.getcwd()[0];
}

function os_env(s) {
	return std.getenv(s);
}

function os_argv() {
	return scriptArgs;
}

function os_cpus_length() {
	// return os.cpus().length;
	return 8;
}

function os_chmod(p, m) {
	if (os_platform() === 'win32') {
		return;
	}
	os_exec("chmod", [m, p]);
}

function os_exit(c) {
	std.exit(c);
}

function crypto_random_uuid() {
	let u = Date.now().toString(16) + Math.random().toString(16) + '0'.repeat(16);
	let guid = [u.substring(0, 8), u.substring(8, 12), '4000-8' + u.substring(13, 16), u.substring(16, 28)].join('-');
	return guid;
}

// https://github.com/kawanet/sha1-uint8array
var SHA1="undefined"!=typeof exports?exports:{};!function(t){var r=[1518500249,1859775393,-1894007588,-899497514],i={sha1:1};SHA1.createHash=function(t){if(t&&!i[t]&&!i[t.toLowerCase()])throw new Error("Digest method not supported");return new s};var n,s=function(){function t(){this.A=1732584193,this.B=-271733879,this.C=-1732584194,this.D=271733878,this.E=-1009589776,this.t=0,this.i=0,(!n||e>=8e3)&&(n=new ArrayBuffer(8e3),e=0),this.h=new Uint8Array(n,e,80),this.o=new Int32Array(n,e,20),e+=80}return t.prototype.update=function(t){if("string"==typeof t)return this.u(t);if(null==t)throw new TypeError("Invalid type: "+typeof t);var r=t.byteOffset,i=t.byteLength,n=i/64|0,s=0;if(n&&!(3&r)&&!(this.t%64)){for(var h=new Int32Array(t.buffer,r,16*n);n--;)this.v(h,s>>2),s+=64;this.t+=s}if(1!==t.BYTES_PER_ELEMENT&&t.buffer){var e=new Uint8Array(t.buffer,r+s,i-s);return this.p(e)}return s===i?this:this.p(t,s)},t.prototype.p=function(t,r){var i=this.h,n=this.o,s=t.length;for(r|=0;r<s;){for(var h=this.t%64,e=h;r<s&&e<64;)i[e++]=t[r++];e>=64&&this.v(n),this.t+=e-h}return this},t.prototype.u=function(t){for(var r=this.h,i=this.o,n=t.length,s=this.i,h=0;h<n;){for(var e=this.t%64,f=e;h<n&&f<64;){var o=0|t.charCodeAt(h++);o<128?r[f++]=o:o<2048?(r[f++]=192|o>>>6,r[f++]=128|63&o):o<55296||o>57343?(r[f++]=224|o>>>12,r[f++]=128|o>>>6&63,r[f++]=128|63&o):s?(o=((1023&s)<<10)+(1023&o)+65536,r[f++]=240|o>>>18,r[f++]=128|o>>>12&63,r[f++]=128|o>>>6&63,r[f++]=128|63&o,s=0):s=o}f>=64&&(this.v(i),i[0]=i[16]),this.t+=f-e}return this.i=s,this},t.prototype.v=function(t,i){var n=this,s=n.A,e=n.B,f=n.C,w=n.D,y=n.E,A=0;for(i|=0;A<16;)h[A++]=o(t[i++]);for(A=16;A<80;A++)h[A]=u(h[A-3]^h[A-8]^h[A-14]^h[A-16]);for(A=0;A<80;A++){var p=A/20|0,d=a(s)+v(p,e,f,w)+y+h[A]+r[p]|0;y=w,w=f,f=c(e),e=s,s=d}this.A=s+this.A|0,this.B=e+this.B|0,this.C=f+this.C|0,this.D=w+this.D|0,this.E=y+this.E|0},t.prototype.digest=function(t){var r=this.h,i=this.o,n=this.t%64|0;for(r[n++]=128;3&n;)r[n++]=0;if((n>>=2)>14){for(;n<16;)i[n++]=0;n=0,this.v(i)}for(;n<16;)i[n++]=0;var s=8*this.t,h=(4294967295&s)>>>0,e=(s-h)/4294967296;return e&&(i[14]=o(e)),h&&(i[15]=o(h)),this.v(i),"hex"===t?this.I():this.U()},t.prototype.I=function(){var t=this,r=t.A,i=t.B,n=t.C,s=t.D,h=t.E;return f(r)+f(i)+f(n)+f(s)+f(h)},t.prototype.U=function(){var t=this,r=t.A,i=t.B,n=t.C,s=t.D,h=t.E,e=t.h,f=t.o;return f[0]=o(r),f[1]=o(i),f[2]=o(n),f[3]=o(s),f[4]=o(h),e.slice(0,20)},t}(),h=new Int32Array(80),e=0,f=function(t){return(t+4294967296).toString(16).substr(-8)},o=254===new Uint8Array(new Uint16Array([65279]).buffer)[0]?function(t){return t}:function(t){return t<<24&4278190080|t<<8&16711680|t>>8&65280|t>>24&255},u=function(t){return t<<1|t>>>31},a=function(t){return t<<5|t>>>27},c=function(t){return t<<30|t>>>2};function v(t,r,i,n){return 0===t?r&i|~r&n:2===t?r&i|r&n|i&n:r^i^n}}();

function uuidv5(path, namespace) {
	const hash = SHA1.createHash('sha1');
	hash.update(namespace);
	hash.update(path);
	const value = hash.digest('hex');
	return value.substring(0, 8) + '-' + value.substring(8, 12) + '-' + value.substring(12, 16) + '-' + value.substring(16, 20) + '-' + value.substring(20, 32);
}

let path_sep = '/';
if (os_platform() === 'win32') {
	path_sep = '\\';
}

let binpath = path_resolve(scriptArgs[0]);
let toolsdir = binpath.substring(0, binpath.lastIndexOf(path_sep));
let __dirname = path_join(toolsdir, "..", "..");

function path_join() {
	let args = Array.from(arguments);
	return path_normalize(args.join(path_sep));
}

function path_isabs(p) {
	return p[0] == "/" || p[1] == ":" || (p[0] == "\\" && p[1] == "\\");
}

function _path_resolve(base, relative) {
	let stack = base.split("/");
    let parts = relative.split("/");
    for (let i = 0; i < parts.length; i++) {
        if (parts[i] == ".")
            continue;
        if (parts[i] == "..")
            stack.pop();
        else
            stack.push(parts[i]);
    }
    return stack.join("/");
}

function path_resolve() {
	let args = Array.from(arguments);
	if (!path_isabs(args[0])) {
		args.unshift(os_cwd());
	}

	let i = args.length - 1;;
	let p = args[i];
	p = path_normalize(p);
	while (!path_isabs(p)) {
		i--;
		p = _path_resolve(args[i], p);
		p = path_normalize(p);
	}
	return p;
}

function path_relative(from, to) {
	let a = from.split(path_sep);
	let b = to.split(path_sep);

	while (a[0] == b[0]) {
		a.shift();
		b.shift();
		if (a.length == 0 || b.length == 0) {
			break;
		}
	}
	let base = "";
	for (let i = 0; i < a.length; ++i) {
		base += ".." + path_sep;
	}
	base += b.join(path_sep);
	return base;
}

function path_normalize(p) {
	while (p.indexOf(path_sep + path_sep) != -1) {
		p = p.replaceAll(path_sep + path_sep, path_sep);
	}
	if (p.endsWith(path_sep)) {
		p = p.substring(0, p.length - 1);
	}
	let ar = p.split(path_sep);
	let i = 0;
	while (i < ar.length) {
		if (i > 0 && ar[i] == ".." && ar[i - 1] != "..") {
			ar.splice(i - 1, 2);
			i--;
		}
		else {
			i++;
		}
	}
	return ar.join(path_sep);
}

function exe_ext() {
	return os_platform() == 'win32' ? '.exe' : '';
}

function icon_run(from, to, width, height, format, background) {
	const exe = path_resolve(toolsdir, 'kraffiti' + exe_ext());
	let params = ['from=' + from, 'to=' + to, 'format=' + format, 'keepaspect'];
	if (width > 0)
		params.push('width=' + width);
	if (height > 0)
		params.push('height=' + height);
	if (background !== undefined)
		params.push('background=' + background.toString(16));
	os_exec(exe, params);
}

function export_ico(icon, to, from) {
	icon_run(path_join(from, icon), to.toString(), 0, 0, 'ico', undefined);
}

function export_png(icon, to, width, height, background, from) {
	icon_run(path_join(from, icon), to.toString(), width, height, 'png', background);
}

function containsDefine(array, value) {
	for (const element of array) {
		if (element.value === value.value && element.config === value.config) {
			return true;
		}
	}
	return false;
}

function containsFancyDefine(array, value) {
	const name = value.value.substring(0, value.value.indexOf('='));
	for (const element of array) {
		if (element.config === value.config) {
			const index = element.value.indexOf('=');
			if (index >= 0) {
				const otherName = element.value.substring(0, index);
				if (name === otherName) {
					return true;
				}
			}
		}
	}
	return false;
}

let scriptdir = '.';

function loadProject(directory, korefile = null) {
	scriptdir = directory;
	if (!korefile) {
		if (fs_exists(path_resolve(directory, 'kfile.js'))) {
			korefile = 'kfile.js';
		}
	}

	globalThis.Project = Project;
	globalThis.platform = Project.platform;
	globalThis.graphics = Options_1.graphics;
	globalThis.__dirname = directory;
	globalThis.targetDirectory = Project.to;
	globalThis.fs_readdir = fs_readdir;
	globalThis.fs_exists = fs_exists;
	globalThis.fs_mtime = fs_mtime;
	globalThis.fs_isdir = fs_isdir;
	globalThis.fs_mkdir = fs_mkdir;
	globalThis.fs_copydir = fs_copydir;
	globalThis.fs_copyfile = fs_copyfile;
	globalThis.fs_writefile = fs_writefile;
	globalThis.fs_readfile = fs_readfile;
	globalThis.fs_ensuredir = fs_ensuredir;
	globalThis.fs_rename = fs_rename;
	globalThis.os_platform = os_platform;
	globalThis.os_env = os_env;
	globalThis.os_argv = os_argv;
	globalThis.os_exec = os_exec;
	globalThis.os_exit = os_exit;
	globalThis.os_cwd = os_cwd;
	globalThis.path_join = path_join;
	globalThis.path_resolve = path_resolve;
	globalThis.path_isabs = path_isabs;
	globalThis.path_sep = path_sep;
	globalThis.path_relative = path_relative;

	return (1, eval)("function _(){" + fs_readfile(path_resolve(directory, korefile)) + "} _();");
}

class Project {
	constructor(name) {
		this.cppStd = '';
		this.cStd = '';
		this.cmdArgs = [];
		this.cFlags = [];
		this.cppFlags = [];
		this.icon = 'icon.png';
		this.additionalBackends = [];
		this.vsdeploy = false;
		this.lto = true;
		this.noFlatten = true;
		this.name = name;
		this.safeName = name.replace(/[^A-z0-9\-\_]/g, '-');
		this.version = '1.0';
		this.debugDir = '';
		this.basedir = scriptdir;
		this.uuid = crypto_random_uuid();
		this.files = [];
		this.customs = [];
		this.javadirs = [];
		this.subProjects = [];
		this.includeDirs = [];
		this.defines = [];
		this.libs = [];
		this.kongDirs = [];
		this.includes = [];
		this.excludes = [];
		this.cppStd = '';
		this.cStd = '';
		this.targetOptions = {
			android: {},
		};
		this.cmd = false;
		this.executableName = null;
	}

	getExecutableName() {
		return this.executableName;
	}

	addBackend(name) {
		this.additionalBackends.push(name);
	}

	findAdditionalBackends(additionalBackends) {
		for (const backend of this.additionalBackends) {
			additionalBackends.push(backend);
		}
		for (let sub of this.subProjects) {
			sub.findAdditionalBackends(additionalBackends);
		}
	}

	findKincProject() {
		if (this.name === 'Kinc') {
			return this;
		}
		for (let sub of this.subProjects) {
			let kinc = sub.findKincProject();
			if (kinc != null) {
				return kinc;
			}
		}
		return null;
	}

	resolveBackends() {
		let additionalBackends = [];
		this.findAdditionalBackends(additionalBackends);
		let kinc = this.findKincProject();
		for (const backend of additionalBackends) {
			kinc.addFile('Backends/' + backend + '/Sources/**', null);
			kinc.addIncludeDir('Backends/' + backend + '/Sources');
		}
	}

	flattenSubProjects() {
		for (let sub of this.subProjects) {
			sub.noFlatten = false;
			sub.flattenSubProjects();
		}
	}

	flatten() {
		this.noFlatten = false;
		this.flattenSubProjects();
	}

	internalFlatten() {
		let out = [];
		for (let sub of this.subProjects)
			sub.internalFlatten();
		for (let sub of this.subProjects) {
			if (sub.noFlatten) {
				out.push(sub);
			}
			else {
				if (sub.cppStd !== '') {
					this.cppStd = sub.cppStd;
				}
				if (sub.cStd !== '') {
					this.cStd = sub.cStd;
				}
				if (sub.cmd) {
					this.cmd = true;
				}
				if (sub.vsdeploy) {
					this.vsdeploy = true;
				}
				if (!sub.lto) {
					this.lto = false;
				}
				if (this.shaderVersion) {
					if (sub.shaderVersion && sub.shaderVersion > this.shaderVersion) {
						this.shaderVersion = sub.shaderVersion;
					}
				}
				else if (sub.shaderVersion) {
					this.shaderVersion = sub.shaderVersion;
				}
				let subbasedir = sub.basedir;
				for (let tkey of Object.keys(sub.targetOptions)) {
					const target = sub.targetOptions[tkey];
					for (let key of Object.keys(target)) {
						const options = this.targetOptions[tkey];
						const option = target[key];
						if (options[key] == null)
							options[key] = option;
						// push library properties to current array instead
						else if (Array.isArray(options[key]) && Array.isArray(option)) {
							for (let value of option) {
								if (!options[key].includes(value))
									options[key].push(value);
							}
						}
					}
				}
				for (let d of sub.defines) {
					if (d.value.indexOf('=') >= 0) {
						if (!containsFancyDefine(this.defines, d)) {
							this.defines.push(d);
						}
					}
					else {
						if (!containsDefine(this.defines, d)) {
							this.defines.push(d);
						}
					}
				}
				for (let file of sub.files) {
					let absolute = file.file;
					if (!path_isabs(absolute)) {
						absolute = path_join(subbasedir, file.file);
					}
					this.files.push({ file: absolute.replace(/\\/g, '/'), options: file.options, projectDir: subbasedir, projectName: sub.name });
				}
				for (const custom of sub.customs) {
					let absolute = custom.file;
					if (!path_isabs(absolute)) {
						absolute = path_join(subbasedir, custom.file);
					}
					this.customs.push({ file: absolute.replace(/\\/g, '/'), command: custom.command, output: custom.output });
				}
				for (let i of sub.includeDirs)
					if (!this.includeDirs.includes(path_resolve(subbasedir, i)))
						this.includeDirs.push(path_resolve(subbasedir, i));
				for (let j of sub.javadirs)
					if (!this.javadirs.includes(path_resolve(subbasedir, j)))
						this.javadirs.push(path_resolve(subbasedir, j));
				for (let k of sub.kongDirs)
					if (!this.kongDirs.includes(path_resolve(subbasedir, k)))
						this.kongDirs.push(path_resolve(subbasedir, k));
				for (let lib of sub.libs) {
					if (lib.indexOf('/') < 0 && lib.indexOf('\\') < 0) {
						if (!this.libs.includes(lib))
							this.libs.push(lib);
					}
					else {
						if (!this.libs.includes(path_resolve(subbasedir, lib)))
							this.libs.push(path_resolve(subbasedir, lib));
					}
				}
				for (let flag of sub.cFlags) {
					if (!this.cFlags.includes(flag)) {
						this.cFlags.push(flag);
					}
				}
				for (let flag of sub.cppFlags) {
					if (!this.cppFlags.includes(flag)) {
						this.cppFlags.push(flag);
					}
				}
			}
		}
		this.subProjects = out;
	}

	getName() {
		return this.name;
	}

	getSafeName() {
		return this.safeName;
	}

	getUuid() {
		return this.uuid;
	}

	matches(text, pattern) {
		const regexstring = pattern.replace(/\./g, '\\.').replace(/\*\*/g, '.?').replace(/\*/g, '[^/]*').replace(/\?/g, '*');
		const regex = new RegExp('^' + regexstring + '$', 'g');
		return regex.test(text);
	}

	matchesAllSubdirs(dir, pattern) {
		if (pattern.endsWith('/**')) {
			return this.matches(this.stringify(dir), pattern.substr(0, pattern.length - 3));
		}
		else
			return false;
	}

	stringify(p) {
		return p.replace(/\\/g, '/');
	}

	addCFlag(flag) {
		this.cFlags.push(flag);
	}

	addCFlags() {
		for (let i = 0; i < arguments.length; ++i) {
			if (typeof arguments[i] === 'string') {
				this.addCFlag(arguments[i]);
			}
		}
	}

	addFileForReal(file, options) {
		for (let index in this.files) {
			if (this.files[index].file === file) {
				this.files[index] = { file: file, options: options, projectDir: this.basedir, projectName: this.name };
				return;
			}
		}
		this.files.push({ file: file, options: options, projectDir: this.basedir, projectName: this.name });
	}

	searchFiles(current) {
		if (current === undefined) {
			for (let sub of this.subProjects)
				sub.searchFiles(undefined);
			this.searchFiles(this.basedir);
			for (let includeobject of this.includes) {
				if (path_isabs(includeobject.file) && includeobject.file.includes('**')) {
					const starIndex = includeobject.file.indexOf('**');
					const endIndex = includeobject.file.substring(0, starIndex).replace(/\\/g, '/').lastIndexOf('/');
					this.searchFiles(includeobject.file.substring(0, endIndex));
				}
				if (includeobject.file.startsWith('../')) {
					let start = '../';
					while (includeobject.file.startsWith(start)) {
						start += '../';
					}
					this.searchFiles(path_resolve(this.basedir, start));
				}
			}
			return;
		}
		let files = fs_readdir(current);
		nextfile: for (let f in files) {
			let file = path_join(current, files[f]);
			let follow = true;
			try {
				if (fs_isdir(file)) {
					follow = false;
				}
			}
			catch (err) {
				follow = false;
			}
			if (!follow) {
				continue;
			}

			file = path_relative(this.basedir, file);
			for (let exclude of this.excludes) {
				if (this.matches(this.stringify(file), exclude)) {
					continue nextfile;
				}
			}
			for (let includeobject of this.includes) {
				let include = includeobject.file;
				if (path_isabs(include)) {
					let inc = include;
					inc = path_relative(this.basedir, inc);
					include = inc;
				}
				if (this.matches(this.stringify(file), this.stringify(include))) {
					this.addFileForReal(this.stringify(file), includeobject.options);
				}
			}
		}
		let dirs = fs_readdir(current);
		nextdir: for (let d of dirs) {
			let dir = path_join(current, d);
			if (d.startsWith('.'))
				continue;
			let follow = true;
			try {
				if (!fs_isdir(dir)) {
					follow = false;
				}
			}
			catch (err) {
				follow = false;
			}
			if (!follow) {
				continue;
			}
			for (let exclude of this.excludes) {
				if (this.matchesAllSubdirs(path_relative(this.basedir, dir), exclude)) {
					continue nextdir;
				}
			}
			this.searchFiles(dir);
		}
	}

	addFile(file, options) {
		this.includes.push({ file: file, options: options });
	}

	addExclude(exclude) {
		this.excludes.push(exclude);
	}

	addDefine(value, config = null) {
		const define = { value, config };
		if (containsDefine(this.defines, define)) {
			return;
		}
		this.defines.push(define);
	}

	addIncludeDir(include) {
		if (this.includeDirs.includes(include))
			return;
		this.includeDirs.push(include);
	}

	addLib(lib) {
		this.libs.push(lib);
	}

	addKongDir(dir) {
		this.kongDirs.push(dir);
		this.addDefine('KINC_KONG');
	}

	getFiles() {
		return this.files;
	}

	getJavaDirs() {
		return this.javadirs;
	}

	getKongDirs() {
		return this.kongDirs;
	}

	getBasedir() {
		return this.basedir;
	}

	getSubProjects() {
		return this.subProjects;
	}

	getIncludeDirs() {
		return this.includeDirs;
	}

	getDefines() {
		return this.defines;
	}

	getLibs() {
		return this.libs;
	}

	getDebugDir() {
		return this.debugDir;
	}

	setDebugDir(debugDir) {
		this.debugDir = path_resolve(this.basedir, debugDir);
		if (!fs_exists(this.debugDir)) {
			throw new Error(`Debug directory ${this.debugDir} does not exist`);
		}
	}

	addProject(directory, projectFile = null) {
		let from = path_isabs(directory) ? directory : path_join(this.basedir, directory);
		if (fs_exists(from) && fs_isdir(from)) {
			const project = loadProject(from, projectFile);
			this.subProjects.push(project);
			return project;
		}
		else {
			throw 'Project directory ' + from + ' not found';
		}
	}

	static create(directory, to, platform, korefile) {
		Project.platform = platform;
		Project.to = path_resolve(to);
		let project = loadProject(path_resolve(directory), korefile);
		let defines = [];
		for (let define of defines) {
			project.addDefine(define);
		}
		return project;
	}

	isCmd() {
		return this.cmd;
	}
}

class Exporter {
	constructor(options) {
		this.path = null;
		this.outFile = null;
	}

	writeFile(file) {
		this.path = file;
		this.outFile = "";
	}

	closeFile() {
		fs_writefile(this.path, this.outFile);
		this.outFile = "";
	}

	p(line = '', indent = 0) {
		let tabs = '';
		for (let i = 0; i < indent; ++i) {
			tabs += '\t';
		}
		this.outFile += tabs + line + '\n';
	}

	nicePath(from, to, filepath) {
		let absolute = filepath;
		if (!path_isabs(absolute)) {
			absolute = path_resolve(from, filepath);
		}
		return path_relative(to, absolute);
	}
}

function isGitPath(p) {
	return p.indexOf('/.git/') >= 0 || p.indexOf('\\.git\\') >= 0 || p.endsWith('/.git') || p.endsWith('\\.git');
}

function getDirFromString(file, base) {
	file = file.replace(/\\/g, '/');
	if (file.indexOf('/') >= 0) {
		let dir = file.substr(0, file.lastIndexOf('/'));
		return path_join(base, path_relative(base, dir)).replace(/\\/g, '/');
	}
	else {
		return base;
	}
}

function getDir(file) {
	return getDirFromString(file.file, file.projectName);
}

function shaderLang(platform) {
	switch (platform) {
		case 'windows':
			switch (Options_1.graphics) {
				case 'opengl':
					return 'glsl';
				case 'direct3d11':
				case 'direct3d12':
				case 'default':
					return 'd3d11';
				case 'vulkan':
					return 'spirv';
			}
		case 'ios':
			switch (Options_1.graphics) {
				case 'default':
				case 'metal':
					return 'metal';
				case 'opengl':
					return 'essl';
			}
		case 'osx':
			switch (Options_1.graphics) {
				case 'default':
				case 'metal':
					return 'metal';
				case 'opengl':
					return 'glsl';
			}
		case 'android':
			switch (Options_1.graphics) {
				case 'default':
				case 'vulkan':
					return 'spirv';
				case 'opengl':
					return 'essl';
			}
		case 'linux':
			switch (Options_1.graphics) {
				case 'default':
				case 'vulkan':
					return 'spirv';
				case 'opengl':
					return 'glsl';
			}
		case 'wasm':
			switch (Options_1.graphics) {
				case 'webgpu':
					return 'spirv';
				case 'opengl':
				case 'default':
					return 'essl';
			}
	}
}

class VisualStudioExporter extends Exporter {
	constructor(options) {
		super(options);
	}

	getDebugDir(from, project) {
		let debugDir = project.getDebugDir();
		if (path_isabs(debugDir)) {
			debugDir = debugDir.replace(/\//g, '\\');
		}
		else {
			debugDir = path_resolve(from, debugDir).replace(/\//g, '\\');
		}
		return debugDir;
	}

	exportUserFile(from, to, project, platform) {
		if (project.getDebugDir() === '')
			return;
		this.writeFile(path_resolve(to, project.getSafeName() + '.vcxproj.user'));
		this.p('<?xml version="1.0" encoding="utf-8"?>');
		this.p('<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">');
		this.p('<PropertyGroup>', 1);
		if (platform === 'windows') {
			this.p('<LocalDebuggerWorkingDirectory>' + this.getDebugDir(from, project) + '</LocalDebuggerWorkingDirectory>', 2);
			this.p('<DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>', 2);
			if (project.cmdArgs.length > 0) {
				this.p('<LocalDebuggerCommandArguments>' + project.cmdArgs.join(' ') + '</LocalDebuggerCommandArguments>', 2);
			}
		}
		this.p('</PropertyGroup>', 1);
		this.p('</Project>');
		this.closeFile();
	}

	writeProjectDeclarations(project, solutionUuid) {
		this.p('Project("{' + solutionUuid.toUpperCase() + '}") = "' + project.getSafeName() + '", "' + project.getSafeName() + '.vcxproj", "{' + project.getUuid().toString().toUpperCase() + '}"');
		if (project.getSubProjects().length > 0) {
			this.p('ProjectSection(ProjectDependencies) = postProject', 1);
			for (let proj of project.getSubProjects()) {
				this.p('{' + proj.getUuid().toString().toUpperCase() + '} = {' + proj.getUuid().toString().toUpperCase() + '}', 2);
			}
			this.p('EndProjectSection', 1);
		}
		this.p('EndProject');
		for (let proj of project.getSubProjects())
			this.writeProjectDeclarations(proj, solutionUuid);
	}

	getConfigs(platform) {
		return ['Debug', 'Develop', 'Release'];
	}

	getSystems(platform) {
		return ['x64'];
	}

	writeProjectBuilds(project, platform) {
		for (let config of this.getConfigs(platform)) {
			for (let system of this.getSystems(platform)) {
				this.p('{' + project.getUuid().toString().toUpperCase() + '}.' + config + '|' + system + '.ActiveCfg = ' + config + '|' + system, 2);
				this.p('{' + project.getUuid().toString().toUpperCase() + '}.' + config + '|' + system + '.Build.0 = ' + config + '|' + system, 2);
				if (project.vsdeploy) {
					this.p('{' + project.getUuid().toString().toUpperCase() + '}.' + config + '|' + system + '.Deploy.0 = ' + config + '|' + system, 2);
				}
			}
		}
		for (let proj of project.getSubProjects())
			this.writeProjectBuilds(proj, platform);
	}

	exportSolution(project, from, to, platform, options) {
		this.writeFile(path_resolve(to, project.getSafeName() + '.sln'));
		if (Options_1.visualstudio === 'vs2022') {
			this.p('Microsoft Visual Studio Solution File, Format Version 12.00');
			this.p('# Visual Studio Version 17');
			this.p('VisualStudioVersion = 17.0.31903.59');
			this.p('MinimumVisualStudioVersion = 10.0.40219.1');
		}
		const solutionUuid = crypto_random_uuid();
		this.writeProjectDeclarations(project, solutionUuid);
		this.p('Global');
		this.p('GlobalSection(SolutionConfigurationPlatforms) = preSolution', 1);
		for (let config of this.getConfigs(platform)) {
			for (let system of this.getSystems(platform)) {
				this.p(config + '|' + system + ' = ' + config + '|' + system, 2);
			}
		}
		this.p('EndGlobalSection', 1);
		this.p('GlobalSection(ProjectConfigurationPlatforms) = postSolution', 1);
		this.writeProjectBuilds(project, platform);
		this.p('EndGlobalSection', 1);
		this.p('GlobalSection(SolutionProperties) = preSolution', 1);
		this.p('HideSolutionNode = FALSE', 2);
		this.p('EndGlobalSection', 1);
		this.p('EndGlobal');
		this.closeFile();
		this.exportProject(from, to, project, platform, project.isCmd(), options);
		this.exportFilters(from, to, project, platform);
		this.exportUserFile(from, to, project, platform);
		if (platform === 'windows') {
			this.exportResourceScript(to);
			export_ico(project.icon, path_resolve(to, 'icon.ico'), from);
		}
	}

	exportManifest(to, project) {
		this.writeFile(path_resolve(to, 'Package.appxmanifest'));
		this.p('<?xml version="1.0" encoding="utf-8"?>');
		this.p('<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10" xmlns:mp="http://schemas.microsoft.com/appx/2014/phone/manifest" xmlns:uap="http://schemas.microsoft.com/appx/manifest/uap/windows10" IgnorableNamespaces="uap mp">');
		this.p('<Identity Name="b2714d6a-f52b-4943-b735-9b5777019bc9" Publisher="CN=Robert" Version="1.0.0.0" />', 1);
		this.p('<mp:PhoneIdentity PhoneProductId="b2714d6a-f52b-4943-b735-9b5777019bc9" PhonePublisherId="00000000-0000-0000-0000-000000000000"/>', 1);
		this.p('<Properties>', 1);
		this.p('<DisplayName>' + project.getName() + '</DisplayName>', 2);
		this.p('<PublisherDisplayName>Robert</PublisherDisplayName>', 2);
		this.p('<Logo>StoreLogo.png</Logo>', 2);
		this.p('</Properties>', 1);
		this.p('<Dependencies>', 1);
		this.p('<TargetDeviceFamily Name="Windows.Universal" MinVersion="10.0.0.0" MaxVersionTested="10.0.0.0" />', 2);
		this.p('</Dependencies>', 1);
		this.p('<Resources>', 1);
		this.p('<Resource Language="x-generate"/>', 2);
		this.p('</Resources>', 1);
		this.p('<Applications>', 1);
		this.p('<Application Id="App" Executable="$targetnametoken$.exe" EntryPoint="' + project.getSafeName() + '.App">', 2);
		this.p('<uap:VisualElements DisplayName="' + project.getName() + '" Square150x150Logo="Logo.png" Square44x44Logo="SmallLogo.png" Description="' + project.getName() + '" BackgroundColor="#464646">', 3);
		this.p('<uap:SplashScreen Image="SplashScreen.png" />', 4);
		this.p('</uap:VisualElements>', 3);
		this.p('</Application>', 2);
		this.p('</Applications>', 1);
		this.p('<Capabilities>', 1);
		this.p('<Capability Name="internetClient" />', 2);
		this.p('</Capabilities>', 1);
		this.p('</Package>');
		this.closeFile();
	}

	exportResourceScript(to) {
		this.writeFile(path_resolve(to, 'resources.rc'));
		this.p('107       ICON         "icon.ico"');
		this.closeFile();
	}

	exportAssetPathFilter(assetPath, dirs, assets) {
		if (isGitPath(assetPath))
			return;
		let dir = getDirFromString(path_join(assetPath, 'whatever'), 'Deployment').trim();
		if (!dirs.includes(dir)) {
			dirs.push(dir);
		}
		let paths = fs_readdir(assetPath);
		for (let p of paths) {
			if (fs_isdir(path_join(assetPath, p)))
				this.exportAssetPathFilter(path_join(assetPath, p), dirs, assets);
			else
				assets.push(path_join(assetPath, p).replace(/\//g, '\\'));
		}
	}

	prettyDir(dir) {
		let prettyDir = dir;
		while (prettyDir.startsWith('../')) {
			prettyDir = prettyDir.substring(3);
		}
		return prettyDir.replace(/\//g, '\\');
	}

	itemGroup(from, to, project, type, prefix, filter) {
		let lastdir = '';
		this.p('<ItemGroup>', 1);
		for (let file of project.getFiles()) {
			let dir = getDir(file);
			if (dir !== lastdir)
				lastdir = dir;
			if (filter(file)) {
				let filepath = '';
				if (project.noFlatten && !path_isabs(file.file)) {
					filepath = path_resolve(path_join(project.basedir, file.file));
				}
				else {
					filepath = this.nicePath(from, to, file.file);
				}
				this.p('<' + type + ' Include="' + filepath + '">', 2);
				this.p('<Filter>' + this.prettyDir(dir) + '</Filter>', 3);
				this.p('</' + type + '>', 2);
			}
		}
		this.p('</ItemGroup>', 1);
	}

	exportFilters(from, to, project, platform) {
		for (let proj of project.getSubProjects())
			this.exportFilters(from, to, proj, platform);
		this.writeFile(path_resolve(to, project.getSafeName() + '.vcxproj.filters'));
		this.p('<?xml version="1.0" encoding="utf-8"?>');
		this.p('<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">');
		let lastdir = '';
		let dirs = [];
		for (let file of project.getFiles()) {
			let dir = getDir(file);
			if (dir !== lastdir) {
				let subdir = dir;
				while (subdir.indexOf('/') >= 0) {
					subdir = subdir.substr(0, subdir.lastIndexOf('/'));
					if (!dirs.includes(subdir))
						dirs.push(subdir);
				}
				dirs.push(dir);
				lastdir = dir;
			}
		}
		let assets = [];
		if (project.vsdeploy)
			this.exportAssetPathFilter(path_resolve(from, project.getDebugDir()), dirs, assets);
		this.p('<ItemGroup>', 1);
		for (let dir of dirs) {
			const pretty = this.prettyDir(dir);
			if (pretty !== '..') {
				this.p('<Filter Include="' + pretty + '">', 2);
				this.p('<UniqueIdentifier>{' + crypto_random_uuid().toString().toUpperCase() + '}</UniqueIdentifier>', 3);
				this.p('</Filter>', 2);
			}
		}
		this.p('</ItemGroup>', 1);
		this.itemGroup(from, to, project, 'ClInclude', () => { }, (file) => {
			return file.file.endsWith('.h') || file.file.endsWith('.hpp');
		});
		this.itemGroup(from, to, project, 'ClCompile', () => { }, (file) => {
			return file.file.endsWith('.cpp') || file.file.endsWith('.c') || file.file.endsWith('.cc') || file.file.endsWith('.cxx');
		});
		this.itemGroup(from, to, project, 'CustomBuild', () => { }, (file) => {
			return file.file.endsWith('.cg') || file.file.endsWith('.hlsl') || file.file.endsWith('.glsl');
		});
		if (project.vsdeploy) {
			lastdir = '';
			this.p('<ItemGroup>', 1);
			for (let file of assets) {
				if (file.indexOf('\\') >= 0 && !isGitPath(file)) {
					let dir = getDirFromString(file, 'Deployment');
					if (dir !== lastdir)
						lastdir = dir;
					this.p('<None Include="' + this.nicePath(from, to, file) + '">', 2);
					this.p('<Filter>' + dir.replace(/\//g, '\\') + '</Filter>', 3);
					this.p('</None>', 2);
				}
			}
			this.p('</ItemGroup>', 1);
		}
		if (platform === 'windows') {
			this.itemGroup(from, to, project, 'ResourceCompile', () => {
				this.p('<None Include="icon.ico">', 2);
				this.p('<Filter>Ressourcendateien</Filter>', 3);
				this.p('</None>', 2);
				this.p('</ItemGroup>', 1);
				this.p('<ItemGroup>', 1);
				this.p('<ResourceCompile Include="resources.rc">', 2);
				this.p('<Filter>Ressourcendateien</Filter>', 3);
				this.p('</ResourceCompile>', 2);
			}, (file) => {
				return file.file.endsWith('.rc');
			});
		}
		this.p('</Project>');
		this.closeFile();
	}

	addPropertyGroup(buildType, wholeProgramOptimization, platform, project, options) {
		this.p('<PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'' + buildType + '|' + this.getSystems(platform)[0] + '\'" Label="Configuration">', 1);
		this.p('<ConfigurationType>Application</ConfigurationType>', 2);
		this.p('<WholeProgramOptimization>' + ((wholeProgramOptimization && project.lto) ? 'true' : 'false') + '</WholeProgramOptimization>', 2);
		this.p('<CharacterSet>MultiByte</CharacterSet>', 2);
		this.p('</PropertyGroup>', 1);
	}

	getPlatformToolset() {
		return 'v143';
	}

	configuration(config, system, indent, project, options) {
		this.p('<PropertyGroup Condition="\'$(Configuration)\'==\'' + config + '\'" Label="Configuration">', indent);
		this.p('<ConfigurationType>Application</ConfigurationType>', indent + 1);
		this.p('<UseDebugLibraries>' + (config === 'Release' ? 'false' : 'true') + '</UseDebugLibraries>', indent + 1);
		this.p('<PlatformToolset>' + this.getPlatformToolset() + '</PlatformToolset>', indent + 1);
		this.p('<PreferredToolArchitecture>x64</PreferredToolArchitecture>', indent + 1);
		if (config === 'Release' && project.lto) {
			this.p('<WholeProgramOptimization>true</WholeProgramOptimization>', indent + 1);
		}
		this.p('<CharacterSet>Unicode</CharacterSet>', indent + 1);
		this.p('</PropertyGroup>', indent);
	}

	getOptimization(config) {
		switch (config) {
			case 'Debug':
			default:
				return 'Disabled';
			case 'Develop':
				return 'Full';
			case 'Release':
				return 'MaxSpeed';
		}
	}

	cStd(project) {
		switch (project.cStd.toLowerCase()) {
			case 'gnu9x':
			case 'gnu99':
			case 'c9x':
			case 'c99':
				return '';
			case 'gnu1x':
			case 'gnu11':
			case 'c1x':
			case 'c11':
				return 'stdc11';
			case 'gnu18':
			case 'gnu17':
			case 'c18':
			case 'c17':
				return 'stdc17';
			case 'gnu2x':
			case 'c2x':
				console.log('C 2x is not yet supported in Visual Studio, using stdc17.');
				return 'stdc17';
		}
	}

	cppStd(project) {
		switch (project.cppStd.toLowerCase()) {
			case 'gnu++03':
			case 'c++03':
			case 'gnu++11':
			case 'c++11':
				return '';
			case 'gnu++14':
			case 'c++14':
				return 'stdcpp14';
			case 'gnu++17':
			case 'c++17':
				return 'stdcpp17';
			case 'gnu++2a':
			case 'c++2a':
			case 'gnu++20':
			case 'c++20':
				return 'stdcpp20';
			case 'gnu++2b':
			case 'c++2b':
			case 'gnu++23':
			case 'c++23':
				console.log('C++ 23 is not yet supported in Visual Studio, using stdcpplatest.');
				return 'stdcpplatest';
		}
	}

	itemDefinition(config, system, includes, debugDefines, releaseDefines, indent, debuglibs, releaselibs, from, project) {
		this.p('<ItemDefinitionGroup Condition="\'$(Configuration)|$(Platform)\'==\'' + config + '|' + system + '\'">', indent);
		this.p('<ClCompile>', indent + 1);
		this.p('<AdditionalIncludeDirectories>' + includes + '</AdditionalIncludeDirectories>', indent + 2);
		this.p('<AdditionalOptions>/bigobj %(AdditionalOptions)</AdditionalOptions>', indent + 2);
		this.p('<WarningLevel>Level3</WarningLevel>', indent + 2);
		this.p('<Optimization>' + this.getOptimization(config) + '</Optimization>', indent + 2);
		if (config === 'Release') {
			this.p('<FunctionLevelLinking>true</FunctionLevelLinking>', indent + 2);
			this.p('<IntrinsicFunctions>true</IntrinsicFunctions>', indent + 2);
		}
		this.p('<PreprocessorDefinitions>' + (config === 'Release' ? releaseDefines : debugDefines) + ((system === 'x64') ? 'SYS_64;' : '') + 'WIN32;_WINDOWS;%(PreprocessorDefinitions)</PreprocessorDefinitions>', indent + 2);
		this.p('<RuntimeLibrary>' + (config === 'Release' ? 'MultiThreaded' : 'MultiThreadedDebug') + '</RuntimeLibrary>', indent + 2);
		this.p('<MultiProcessorCompilation>true</MultiProcessorCompilation>', indent + 2);
		this.p('<MinimalRebuild>false</MinimalRebuild>', indent + 2);
		if (config === 'Develop') {
			this.p('<BasicRuntimeChecks>Default</BasicRuntimeChecks>', indent + 2);
		}
		if (project.cStd !== '') {
			const cStd = this.cStd(project);
			if (cStd !== '') {
				this.p('<LanguageStandard_C>' + cStd + '</LanguageStandard_C>', indent + 2);
			}
		}
		if (project.cppStd !== '') {
			const cppStd = this.cppStd(project);
			if (cppStd !== '') {
				this.p('<LanguageStandard>' + cppStd + '</LanguageStandard>', indent + 2);
			}
		}
		this.p('</ClCompile>', indent + 1);
		this.p('<Link>', indent + 1);
		if (project.isCmd())
			this.p('<SubSystem>Console</SubSystem>', indent + 2);
		else
			this.p('<SubSystem>Windows</SubSystem>', indent + 2);
		this.p('<GenerateDebugInformation>true</GenerateDebugInformation>', indent + 2);
		if (config === 'Release') {
			this.p('<EnableCOMDATFolding>true</EnableCOMDATFolding>', indent + 2);
			this.p('<OptimizeReferences>true</OptimizeReferences>', indent + 2);
		}

		let libs = config === 'Release' ? releaselibs : debuglibs;
		this.p('<AdditionalDependencies>' + libs + 'kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)</AdditionalDependencies>', indent + 2);
		this.p('</Link>', indent + 1);
		this.p('<Manifest>', indent + 1);
		this.p('<EnableDpiAwareness>PerMonitorHighDPIAware</EnableDpiAwareness>', indent + 2);
		this.p('</Manifest>', indent + 1);
		this.p('</ItemDefinitionGroup>', indent);
	}

	windowsSDKs() {
		// Environment* env = Environment::GetCurrent(args);
		// Isolate* isolate = env->isolate();
		// std::vector<Local<Value>> result;
		// HKEY key;
		// LSTATUS status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots", 0, KEY_READ | KEY_QUERY_VALUE, &key);
		// char name[256];
		// DWORD nameLength;
		// for (DWORD i = 0;; ++i) {
		// 	nameLength = sizeof(name);
		// 	status = RegEnumKeyExA(key, i, &name[0], &nameLength, NULL, NULL, NULL, NULL);
		// 	if (status != ERROR_SUCCESS) {
		// 	break;
		// 	}
		// 	result.emplace_back(String::NewFromUtf8(isolate, name).ToLocalChecked());
		// }
		// RegCloseKey(key);
		// args.GetReturnValue().Set(Array::New(isolate, result.data(), result.size()));
	}

	findWindowsSdk() {
		const sdks = windowsSDKs();
		let best = [0, 0, 0, 0];
		for (let key of sdks) {
			let elements = key.split('\\');
			let last = elements[elements.length - 1];
			if (last.indexOf('.') >= 0) {
				let numstrings = last.split('.');
				let nums = [];
				for (let str of numstrings) {
					nums.push(parseInt(str));
				}
				if (nums[0] > best[0]) {
					best = nums;
				}
				else if (nums[0] === best[0]) {
					if (nums[1] > best[1]) {
						best = nums;
					}
					else if (nums[1] === best[1]) {
						if (nums[2] > best[2]) {
							best = nums;
						}
						else if (nums[2] === best[2]) {
							if (nums[3] > best[3]) {
								best = nums;
							}
						}
					}
				}
			}
		}
		if (best[0] > 0) {
			return best[0] + '.' + best[1] + '.' + best[2] + '.' + best[3];
		}
		else {
			return null;
		}
	}

	globals(platform, indent) {
		if (Options_1.visualstudio === 'vs2022') {
			this.p('<VCProjectVersion>16.0</VCProjectVersion>', indent);
			this.p('<WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>', indent);
		}
	}

	extensionSettings(indent) {
		this.p('<Import Project="$(VCTargetsPath)\\BuildCustomizations\\masm.props" />');
	}

	extensionTargets(indent) {
		this.p('<Import Project="$(VCTargetsPath)\\BuildCustomizations\\masm.targets"/>', indent);
	}

	exportProject(from, to, project, platform, cmd, options) {
		for (let proj of project.getSubProjects()) {
			this.exportProject(from, to, proj, platform, cmd, options);
		}
		this.writeFile(path_resolve(to, project.getSafeName() + '.vcxproj'));
		this.p('<?xml version="1.0" encoding="utf-8"?>');
		this.p('<Project DefaultTargets="Build" ' + 'xmlns="http://schemas.microsoft.com/developer/msbuild/2003">');
		this.p('<ItemGroup Label="ProjectConfigurations">', 1);
		for (let system of this.getSystems(platform)) {
			for (let config of this.getConfigs(platform)) {
				this.p('<ProjectConfiguration Include="' + config + '|' + system + '">', 2);
				this.p('<Configuration>' + config + '</Configuration>', 3);
				this.p('<Platform>' + system + '</Platform>', 3);
				this.p('</ProjectConfiguration>', 2);
			}
		}
		this.p('</ItemGroup>', 1);
		this.p('<PropertyGroup Label="Globals">', 1);
		this.p('<ProjectGuid>{' + project.getUuid().toString().toUpperCase() + '}</ProjectGuid>', 2);
		this.globals(platform, 2);
		this.p('</PropertyGroup>', 1);
		this.p('<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />', 1);
		for (let config of this.getConfigs(platform)) {
			for (let system of this.getSystems(platform)) {
				this.configuration(config, system, 1, project, options);
			}
		}
		this.p('<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />', 1);
		this.p('<ImportGroup Label="ExtensionSettings">', 1);
		this.extensionSettings(2);
		this.p('</ImportGroup>', 1);
		this.p('<PropertyGroup Label="UserMacros" />', 1);
		if (project.getExecutableName()) {
			this.p('<PropertyGroup>', 1);
			this.p('<TargetName>' + project.getExecutableName() + '</TargetName>', 2);
			this.p('</PropertyGroup>', 1);
		}
		if (platform === 'windows') {
			for (let system of this.getSystems(platform)) {
				this.p('<ImportGroup Label="PropertySheets" Condition="\'$(Platform)\'==\'' + system + '\'">', 1);
				this.p('<Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists(\'$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props\')" Label="LocalAppDataPlatform" />', 2);
				this.p('</ImportGroup>', 1);
			}
		}
		let debugDefines = '_DEBUG;';
		let releaseDefines = 'NDEBUG;';
		for (const define of project.getDefines()) {
			if (define.config && define.config.toLowerCase() === 'debug') {
				debugDefines += define.value + ';';
			}
			else if (define.config && define.config.toLowerCase() === 'release') {
				releaseDefines += define.value + ';';
			}
			else {
				debugDefines += define.value + ';';
				releaseDefines += define.value + ';';
			}
		}
		let incstring = '';
		let includeDirs = project.getIncludeDirs();
		for (let include of includeDirs) {
			let relativized = path_relative(to, path_resolve(from, include));
			if (relativized === '') {
				relativized = '.';
			}
			incstring += relativized + ';';
		}
		if (incstring.length > 0)
			incstring = incstring.substr(0, incstring.length - 1);
		let debuglibs = '';
		for (let proj of project.getSubProjects()) {
			if (proj.noFlatten) {
				debuglibs += project.basedir + '\\build\\x64\\Debug\\' + proj.getSafeName() + '.lib;';
			}
			else {
				debuglibs += 'Debug\\' + proj.getSafeName() + '.lib;';
			}
		}
		for (let lib of project.getLibs()) {
			if (fs_exists(path_resolve(from, lib + '.lib'))) {
				debuglibs += path_relative(to, path_resolve(from, lib)) + '.lib;';
			}
			else {
				debuglibs += lib + '.lib;';
			}
		}
		let releaselibs = '';
		for (let proj of project.getSubProjects()) {
			if (proj.noFlatten) {
				releaselibs += project.basedir + '\\build\\x64\\Release\\' + proj.getSafeName() + '.lib;';
			}
			else {
				releaselibs += 'Release\\' + proj.getSafeName() + '.lib;';
			}
		}
		for (let proj of project.getSubProjects())
			releaselibs += 'Release\\' + proj.getSafeName() + '.lib;';
		for (let lib of project.getLibs()) {
			if (fs_exists(path_resolve(from, lib + '.lib'))) {
				releaselibs += path_relative(to, path_resolve(from, lib)) + '.lib;';
			}
			else {
				releaselibs += lib + '.lib;';
			}
		}
		for (let config of this.getConfigs(platform)) {
			for (let system of this.getSystems(platform)) {
				this.itemDefinition(config, system, incstring, debugDefines, releaseDefines, 2, debuglibs, releaselibs, from, project);
			}
		}
		this.p('<ItemGroup>', 1);
		for (let file of project.getFiles()) {
			let filepath = '';
			if (project.noFlatten && !path_isabs(file.file)) {
				filepath = path_resolve(project.basedir + '/' + file.file);
			}
			else {
				filepath = this.nicePath(from, to, file.file);
			}
			if (file.file.endsWith('.h') || file.file.endsWith('.hpp'))
				this.p('<ClInclude Include="' + filepath + '" />', 2);
		}
		this.p('</ItemGroup>', 1);
		if (project.vsdeploy) {
			this.p('<ItemGroup>', 1);
			this.exportAssetPath(project, from, to, path_resolve(from, project.getDebugDir()));
			this.p('</ItemGroup>', 1);
		}
		this.p('<ItemGroup>', 1);
		let objects = {};
		for (let fileobject of project.getFiles()) {
			let file = fileobject.file;
			if (file.endsWith('.cpp') || file.endsWith('.c') || file.endsWith('cc') || file.endsWith('cxx')) {
				let name = file.toLowerCase();
				if (name.indexOf('/') >= 0)
					name = name.substr(name.lastIndexOf('/') + 1);
				name = name.substr(0, name.lastIndexOf('.'));
				let filepath = '';
				if (project.noFlatten && !path_isabs(file)) {
					filepath = path_resolve(project.basedir + '/' + file);
				}
				else {
					filepath = this.nicePath(from, to, file);
				}
				if (!objects[name]) {
					this.p('<ClCompile Include="' + filepath + '" />', 2);
					objects[name] = true;
				}
				else {
					while (objects[name]) {
						name = name + '_';
					}
					this.p('<ClCompile Include="' + filepath + '">', 2);
					this.p('<ObjectFileName>$(IntDir)\\' + name + '.obj</ObjectFileName>', 3);
					this.p('</ClCompile>', 2);
					objects[name] = true;
				}
			}
		}
		this.p('</ItemGroup>', 1);
		this.p('<ItemGroup>', 1);
		for (let file of project.getFiles()) {
			if (file.file.endsWith('.natvis')) {
				this.p('<Natvis Include="' + this.nicePath(from, to, file.file) + '"/>', 2);
			}
		}
		this.p('</ItemGroup>', 1);
		if (platform === 'windows') {
			this.p('<ItemGroup>', 1);
			for (let file of project.getFiles()) {
				if (file.file.endsWith('.glsl')) {
					this.p('<CustomBuild Include="' + this.nicePath(from, to, file.file) + '">', 2);
					this.p('<FileType>Document</FileType>', 2);
					const shaderDir = path_isabs(project.getDebugDir()) ? project.getDebugDir() : path_join(from, project.getDebugDir());
					const krafix = path_join(toolsdir, 'krafix.exe');
					this.p('<Command>"' + path_relative(to, krafix) + '" ' + shaderLang('windows') + ' "%(FullPath)" ' + path_relative(to, path_join(shaderDir, '%(Filename)')).replace(/\//g, '\\') + ' .\\ ' + platform + ' --quiet</Command>', 2);
					this.p('<Outputs>' + path_relative(to, path_join(shaderDir, '%(Filename)')).replace(/\//g, '\\') + ';%(Outputs)</Outputs>', 2);
					this.p('<Message>%(Filename)%(Extension)</Message>', 2);
					this.p('</CustomBuild>', 2);
				}
			}
			this.p('</ItemGroup>', 1);
			this.p('<ItemGroup>', 1);
			for (let file of project.customs) {
				this.p('<CustomBuild Include="' + this.nicePath(from, to, file.file) + '">', 2);
				this.p('<FileType>Document</FileType>', 2);
				this.p('<Command>' + file.command + '</Command>', 2);
				this.p('<Outputs>' + file.output + '</Outputs>', 2);
				this.p('<Message>%(Filename)%(Extension)</Message>', 2);
				this.p('</CustomBuild>', 2);
			}
			this.p('</ItemGroup>');
			this.p('<ItemGroup>', 1);
			this.p('<None Include="icon.ico" />', 2);
			this.p('</ItemGroup>', 1);
			this.p('<ItemGroup>', 1);
			this.p('<ResourceCompile Include="resources.rc" />', 2);
			for (let file of project.getFiles()) {
				if (file.file.endsWith('.rc')) {
					this.p('<ResourceCompile Include="' + this.nicePath(from, to, file.file) + '" />', 2);
				}
			}
			this.p('</ItemGroup>', 1);
		}
		this.p('<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />', 1);
		this.p('<ImportGroup Label="ExtensionTargets">', 1);
		this.extensionTargets(2);
		this.p('</ImportGroup>', 1);
		this.p('</Project>');
		this.closeFile();
	}

	exportAssetPath(project, from, to, assetPath) {
		if (isGitPath(assetPath))
			return;
		let paths = fs_readdir(assetPath);
		for (let p of paths) {
			if (isGitPath(p))
				continue;
			if (fs_isdir(path_join(assetPath, p))) {
				this.exportAssetPath(project, from, to, path_join(assetPath, p));
			}
			else {
				this.p('<None Include="' + this.nicePath(from, to, path_join(assetPath, p)) + '">', 2);
				this.p('<DeploymentContent>true</DeploymentContent>', 3);
				this.p('<Link>' + path_relative(project.getDebugDir(), path_join(assetPath, p)) + '</Link>', 3);
				this.p('</None>', 2);
			}
		}
	}
}

class WasmExporter extends Exporter {
	constructor(options) {
		super(options);
		this.compileCommands = new CompilerCommandsExporter(options);
		const compiler = 'clang';
		const compilerFlags = '--target=wasm32 -nostdlib -matomics -mbulk-memory';
		this.make = new MakeExporter(options, compiler, compiler, compilerFlags, compilerFlags, '--target=wasm32 -nostdlib -matomics -mbulk-memory "-Wl,--import-memory,--shared-memory"', '.wasm');
	}

	exportSolution(project, from, to, platform, options) {
		this.make.exportSolution(project, from, to, platform, options);
		this.compileCommands.exportSolution(project, from, to, platform, options);
	}
}

function newPathId(path) {
	return uuidv5(path, '7448ebd8-cfc8-4f45-8b3d-5df577ceea6d').toUpperCase();
}

function getDir2(file) {
	if (file.file.indexOf('/') >= 0) {
		let dir = file.file.substr(0, file.file.lastIndexOf('/'));
		return path_join(file.projectName, path_relative(file.projectDir, dir)).replace(/\\/g, '/');
	}
	else {
		return file.projectName;
	}
}

class Directory {
	constructor(dirname) {
		this.dirname = dirname;
		this.id = newPathId(dirname);
	}

	getName() {
		return this.dirname;
	}

	getLastName() {
		if (this.dirname.indexOf('/') < 0)
			return this.dirname;
		return this.dirname.substr(this.dirname.lastIndexOf('/') + 1);
	}

	getId() {
		return this.id;
	}
}

class File {
	constructor(filename, dir) {
		this.filename = filename;
		this.dir = dir;
		this.buildid = newPathId(dir + filename + '_buildid');
		this.fileid = newPathId(dir + filename + '_fileid');
	}

	getBuildId() {
		return this.buildid;
	}

	getFileId() {
		return this.fileid;
	}

	isBuildFile() {
		return this.filename.endsWith('.c') || this.filename.endsWith('.cpp') || this.filename.endsWith('.m') || this.filename.endsWith('.mm') || this.filename.endsWith('.cc') || this.filename.endsWith('.metal') || this.filename.endsWith('.storyboard');
	}

	getName() {
		return this.filename;
	}

	getLastName() {
		if (this.filename.indexOf('/') < 0)
			return this.filename;
		return this.filename.substr(this.filename.lastIndexOf('/') + 1);
	}

	getDir() {
		return this.dir;
	}

	toString() {
		return this.getName();
	}
}

class Framework {
	constructor(name) {
		this.name = name;
		this.buildid = newPathId(name + '_buildid');
		this.fileid = newPathId(name + '_fileid');
		this.localPath = null;
	}

	toString() {
		if (this.name.indexOf('.') < 0)
			return this.name + '.framework';
		else
			return this.name;
	}

	getBuildId() {
		return this.buildid.toString().toUpperCase();
	}

	getFileId() {
		return this.fileid.toString().toUpperCase();
	}
}

function findDirectory(dirname, directories) {
	for (let dir of directories) {
		if (dir.getName() === dirname) {
			return dir;
		}
	}
	return null;
}

function addDirectory(dirname, directories) {
	let dir = findDirectory(dirname, directories);
	if (dir === null) {
		dir = new Directory(dirname);
		directories.push(dir);
		while (dirname.indexOf('/') >= 0) {
			dirname = dirname.substr(0, dirname.lastIndexOf('/'));
			addDirectory(dirname, directories);
		}
	}
	return dir;
}

class IconImage {
	constructor(idiom, size, scale, background = undefined) {
		this.idiom = idiom;
		this.size = size;
		this.scale = scale;
		this.background = background;
	}
}

class XCodeExporter extends Exporter {
	constructor(options) {
		super(options);
	}

	exportWorkspace(to, project) {
		const dir = path_resolve(to, project.getSafeName() + '.xcodeproj', 'project.xcworkspace');
		fs_ensuredir(dir);
		this.writeFile(path_resolve(to, project.getSafeName() + '.xcodeproj', 'project.xcworkspace', 'contents.xcworkspacedata'));
		this.p('<?xml version="1.0" encoding="UTF-8"?>');
		this.p('<Workspace');
		this.p('version = "1.0">');
		this.p('<FileRef');
		this.p('location = "self:' + project.getSafeName() + '.xcodeproj">');
		this.p('</FileRef>');
		this.p('</Workspace>');
		this.closeFile();
	}

	exportSolution(project, from, to, platform, options) {
		const xdir = path_resolve(to, project.getSafeName() + '.xcodeproj');
		fs_ensuredir(xdir);
		this.exportWorkspace(to, project);
		function add_icons(icons, idiom, sizes, scales) {
			for (let i = 0; i < sizes.length; ++i) {
				icons.push(new IconImage(idiom, sizes[i], scales[i]));
			}
		}
		let icons = [];
		if (platform === 'ios') {
			add_icons(icons, 'iphone', [20, 20, 29, 29, 40, 40, 60, 60], [2, 3, 2, 3, 2, 3, 2, 3]);
			add_icons(icons, 'ipad', [20, 20, 29, 29, 40, 40, 76, 76, 83.5], [1, 2, 1, 2, 1, 2, 1, 2, 2]);
			icons.push(new IconImage('ios-marketing', 1024, 1, 0x000000ff));
		}
		else {
			add_icons(icons, 'mac', [16, 16, 32, 32, 128, 128, 256, 256, 512, 512], [1, 2, 1, 2, 1, 2, 1, 2, 1, 2]);
		}
		const iconsdir = path_resolve(to, 'Images.xcassets', 'AppIcon.appiconset');
		fs_ensuredir(iconsdir);
		this.writeFile(path_resolve(to, 'Images.xcassets', 'AppIcon.appiconset', 'Contents.json'));
		this.p('{');
		this.p('"images" : [', 1);
		for (let i = 0; i < icons.length; ++i) {
			const icon = icons[i];
			this.p('{', 2);
			this.p('"idiom" : "' + icon.idiom + '",', 3);
			this.p('"size" : "' + icon.size + 'x' + icon.size + '",', 3);
			this.p('"filename" : "' + icon.idiom + icon.scale + 'x' + icon.size + '.png",', 3);
			this.p('"scale" : "' + icon.scale + 'x"', 3);
			if (i === icons.length - 1)
				this.p('}', 2);
			else
				this.p('},', 2);
		}
		this.p('],', 1);
		this.p('"info" : {', 1);
		this.p('"version" : 1,', 2);
		this.p('"author" : "xcode"', 2);
		this.p('}', 1);
		this.p('}');
		this.closeFile();
		for (let i = 0; i < icons.length; ++i) {
			const icon = icons[i];
			export_png(project.icon, path_resolve(to, 'Images.xcassets', 'AppIcon.appiconset', icon.idiom + icon.scale + 'x' + icon.size + '.png'), icon.size * icon.scale, icon.size * icon.scale, icon.background, from);
		}
		let plistname = '';
		let files = [];
		let directories = [];
		for (let fileobject of project.getFiles()) {
			let filename = fileobject.file;
			if (filename.endsWith('.plist'))
				plistname = filename;
			let dir = addDirectory(getDir2(fileobject), directories);
			let file = new File(filename, dir);
			files.push(file);
		}
		if (plistname.length === 0)
			throw 'no plist found';
		let frameworks = [];
		for (let lib of project.getLibs()) {
			frameworks.push(new Framework(lib));
		}
		let targetOptions = {
			bundle: 'tech.kode.$(PRODUCT_NAME:rfc1034identifier)',
			version: '1.0',
			build: '1',
			organizationName: 'the Kore Development Team',
			developmentTeam: ''
		};
		if (project.targetOptions && project.targetOptions.ios) {
			let userOptions = project.targetOptions.ios;
			if (userOptions.bundle)
				targetOptions.bundle = userOptions.bundle;
			if (userOptions.version)
				targetOptions.version = userOptions.version;
			if (userOptions.build)
				targetOptions.build = userOptions.build;
			if (userOptions.organizationName)
				targetOptions.organizationName = userOptions.organizationName;
			if (userOptions.developmentTeam)
				targetOptions.developmentTeam = userOptions.developmentTeam;
		}
		const projectId = newPathId('_projectId');
		const appFileId = newPathId('_appFileId');
		const frameworkBuildId = newPathId('_frameworkBuildId');
		const sourceBuildId = newPathId('_sourceBuildId');
		const frameworksGroupId = newPathId('_frameworksGroupId');
		const productsGroupId = newPathId('_productsGroupId');
		const mainGroupId = newPathId('_mainGroupId');
		const targetId = newPathId('_targetId');
		const nativeBuildConfigListId = newPathId('_nativeBuildConfigListId');
		const projectBuildConfigListId = newPathId('_projectBuildConfigListId');
		const debugId = newPathId('_debugId');
		const releaseId = newPathId('_releaseId');
		const nativeDebugId = newPathId('_nativeDebugId');
		const nativeReleaseId = newPathId('_nativeReleaseId');
		const debugDirFileId = newPathId('_debugDirFileId');
		const debugDirBuildId = newPathId('_debugDirBuildId');
		const resourcesBuildId = newPathId('_resourcesBuildId');
		const iconFileId = newPathId('_iconFileId');
		const iconBuildId = newPathId('_iconBuildId');
		this.writeFile(path_resolve(to, project.getSafeName() + '.xcodeproj', 'project.pbxproj'));
		this.p('// !$*UTF8*$!');
		this.p('{');
		this.p('archiveVersion = 1;', 1);
		this.p('classes = {', 1);
		this.p('};', 1);
		this.p('objectVersion = 46;', 1);
		this.p('objects = {', 1);
		this.p();
		this.p('/* Begin PBXBuildFile section */');
		for (let framework of frameworks) {
			this.p(framework.getBuildId() + ' /* ' + framework.toString() + ' in Frameworks */ = {isa = PBXBuildFile; fileRef = ' + framework.getFileId() + ' /* ' + framework.toString() + ' */; };', 2);
		}
		this.p(debugDirBuildId + ' /* Deployment in Resources */ = {isa = PBXBuildFile; fileRef = ' + debugDirFileId + ' /* Deployment */; };', 2);
		for (let file of files) {
			if (file.isBuildFile()) {
				this.p(file.getBuildId() + ' /* ' + file.toString() + ' in Sources */ = {isa = PBXBuildFile; fileRef = ' + file.getFileId() + ' /* ' + file.toString() + ' */; };', 2);
			}
		}
		this.p(iconBuildId + ' /* Images.xcassets in Resources */ = {isa = PBXBuildFile; fileRef = ' + iconFileId + ' /* Images.xcassets */; };', 2);
		this.p('/* End PBXBuildFile section */');
		this.p();
		this.p('/* Begin PBXFileReference section */');
		let executableName = project.getSafeName();
		if (project.getExecutableName()) {
			executableName = project.getExecutableName();
		}
		this.p(appFileId + ' /* ' + project.getSafeName() + '.app */ = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = "' + executableName + '.app"; sourceTree = BUILT_PRODUCTS_DIR; };', 2);
		for (let framework of frameworks) {
			if (framework.toString().endsWith('.framework')) {
				// Local framework - a directory is specified
				if (framework.toString().indexOf('/') >= 0) {
					framework.localPath = path_resolve(from, framework.toString());
					this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = ' + framework.toString() + '; path = ' + framework.localPath + '; sourceTree = "<absolute>"; };', 2);
				}
				// XCode framework
				else {
					this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */ = {isa = PBXFileReference; lastKnownFileType = wrapper.framework; name = ' + framework.toString() + '; path = System/Library/Frameworks/' + framework.toString() + '; sourceTree = SDKROOT; };', 2);
				}
			}
			else if (framework.toString().endsWith('.dylib')) {
				// Local dylib, e.g. V8 in Krom - a directory is specified
				if (framework.toString().indexOf('/') >= 0) {
					framework.localPath = path_resolve(from, framework.toString());
					this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */ = {isa = PBXFileReference; lastKnownFileType = compiled.mach-o.dylib; name = ' + framework.toString() + '; path = ' + framework.localPath + '; sourceTree = "<absolute>"; };', 2);
				}
				else {
					this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */ = {isa = PBXFileReference; lastKnownFileType = compiled.mach-o.dylib; name = ' + framework.toString() + '; path = usr/lib/' + framework.toString() + '; sourceTree = SDKROOT; };', 2);
				}
			}
			else {
				framework.localPath = path_resolve(from, framework.toString());
				this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */ = {isa = PBXFileReference; lastKnownFileType = archive.ar; name = ' + framework.toString() + '; path = ' + framework.localPath + '; sourceTree = "<group>"; };', 2);
			}
		}
		this.p(debugDirFileId + ' /* Deployment */ = {isa = PBXFileReference; lastKnownFileType = folder; name = Deployment; path = "' + path_resolve(from, project.getDebugDir()) + '"; sourceTree = "<group>"; };', 2);
		for (let file of files) {
			let filetype = 'unknown';
			let fileencoding = '';
			if (file.getName().endsWith('.storyboard'))
				filetype = 'file.storyboard';
			if (file.getName().endsWith('.plist'))
				filetype = 'text.plist.xml';
			if (file.getName().endsWith('.h'))
				filetype = 'sourcecode.c.h';
			if (file.getName().endsWith('.m'))
				filetype = 'sourcecode.c.objc';
			if (file.getName().endsWith('.c'))
				filetype = 'sourcecode.c.c';
			if (file.getName().endsWith('.cpp'))
				filetype = 'sourcecode.c.cpp';
			if (file.getName().endsWith('.cc'))
				filetype = 'sourcecode.c.cpp';
			if (file.getName().endsWith('.mm'))
				filetype = 'sourcecode.c.objcpp';
			if (file.getName().endsWith('.metal')) {
				filetype = 'sourcecode.metal';
				fileencoding = 'fileEncoding = 4; ';
			}
			if (!file.getName().endsWith('.DS_Store')) {
				this.p(file.getFileId() + ' /* ' + file.toString() + ' */ = {isa = PBXFileReference; ' + fileencoding + 'lastKnownFileType = ' + filetype + '; name = "' + file.getLastName() + '"; path = "' + path_resolve(from, file.toString()) + '"; sourceTree = "<group>"; };', 2);
			}
		}
		this.p(iconFileId + ' /* Images.xcassets */ = {isa = PBXFileReference; lastKnownFileType = folder.assetcatalog; path = Images.xcassets; sourceTree = "<group>"; };', 2);
		this.p('/* End PBXFileReference section */');
		this.p();
		this.p('/* Begin PBXFrameworksBuildPhase section */');
		this.p(frameworkBuildId + ' /* Frameworks */ = {', 2);
		this.p('isa = PBXFrameworksBuildPhase;', 3);
		this.p('buildActionMask = 2147483647;', 3);
		this.p('files = (', 3);
		for (let framework of frameworks) {
			this.p(framework.getBuildId() + ' /* ' + framework.toString() + ' in Frameworks */,', 4);
		}
		this.p(');', 3);
		this.p('runOnlyForDeploymentPostprocessing = 0;', 3);
		this.p('};', 2);
		this.p('/* End PBXFrameworksBuildPhase section */');
		this.p();
		this.p('/* Begin PBXGroup section */');
		this.p(mainGroupId + ' = {', 2);
		this.p('isa = PBXGroup;', 3);
		this.p('children = (', 3);
		this.p(iconFileId + ' /* Images.xcassets */,', 4);
		this.p(debugDirFileId + ' /* Deployment */,', 4);
		for (let dir of directories) {
			if (dir.getName().indexOf('/') < 0)
				this.p(dir.getId() + ' /* ' + dir.getName() + ' */,', 4);
		}
		this.p(frameworksGroupId + ' /* Frameworks */,', 4);
		this.p(productsGroupId + ' /* Products */,', 4);
		this.p(');', 3);
		this.p('sourceTree = "<group>";', 3);
		this.p('};', 2);
		this.p(productsGroupId + ' /* Products */ = {', 2);
		this.p('isa = PBXGroup;', 3);
		this.p('children = (', 3);
		this.p(appFileId + ' /* ' + project.getSafeName() + '.app */,', 4);
		this.p(');', 3);
		this.p('name = Products;', 3);
		this.p('sourceTree = "<group>";', 3);
		this.p('};', 2);
		this.p(frameworksGroupId + ' /* Frameworks */ = {', 2);
		this.p('isa = PBXGroup;', 3);
		this.p('children = (', 3);
		for (let framework of frameworks) {
			this.p(framework.getFileId() + ' /* ' + framework.toString() + ' */,', 4);
		}
		this.p(');', 3);
		this.p('name = Frameworks;', 3);
		this.p('sourceTree = "<group>";', 3);
		this.p('};', 2);
		for (let dir of directories) {
			this.p(dir.getId() + ' /* ' + dir.getName() + ' */ = {', 2);
			this.p('isa = PBXGroup;', 3);
			this.p('children = (', 3);
			for (let dir2 of directories) {
				if (dir2 === dir)
					continue;
				if (dir2.getName().startsWith(dir.getName())) {
					if (dir2.getName().substr(dir.getName().length + 1).indexOf('/') < 0)
						this.p(dir2.getId() + ' /* ' + dir2.getName() + ' */,', 4);
				}
			}
			for (let file of files) {
				if (file.getDir() === dir && !file.getName().endsWith('.DS_Store'))
					this.p(file.getFileId() + ' /* ' + file.toString() + ' */,', 4);
			}
			this.p(');', 3);
			if (dir.getName().indexOf('/') < 0) {
				this.p('path = ../;', 3);
				this.p('name = "' + dir.getLastName() + '";', 3);
			}
			else
				this.p('name = "' + dir.getLastName() + '";', 3);
			this.p('sourceTree = "<group>";', 3);
			this.p('};', 2);
		}
		this.p('/* End PBXGroup section */');
		this.p();
		this.p('/* Begin PBXNativeTarget section */');
		this.p(targetId + ' /* ' + project.getSafeName() + ' */ = {', 2);
		this.p('isa = PBXNativeTarget;', 3);
		this.p('buildConfigurationList = ' + nativeBuildConfigListId + ' /* Build configuration list for PBXNativeTarget "' + project.getSafeName() + '" */;', 3);
		this.p('buildPhases = (', 3);
		this.p(sourceBuildId + ' /* Sources */,', 4);
		this.p(frameworkBuildId + ' /* Frameworks */,', 4);
		this.p(resourcesBuildId + ' /* Resources */,', 4);
		this.p(');', 3);
		this.p('buildRules = (', 3);
		this.p(');', 3);
		this.p('dependencies = (', 3);
		this.p(');', 3);
		this.p('name = "' + project.getName() + '";', 3);
		this.p('productName = "' + project.getName() + '";', 3);
		this.p('productReference = ' + appFileId + ' /* ' + project.getSafeName() + '.app */;', 3);
		this.p('productType = "com.apple.product-type.' + (project.isCmd() ? 'tool' : 'application') + '";', 3);
		this.p('};', 2);
		this.p('/* End PBXNativeTarget section */');
		this.p();
		this.p('/* Begin PBXProject section */');
		this.p(projectId + ' /* Project object */ = {', 2);
		this.p('isa = PBXProject;', 3);
		this.p('attributes = {', 3);
		this.p('LastUpgradeCheck = 1230;', 4);
		this.p('ORGANIZATIONNAME = "' + targetOptions.organizationName + '";', 4);
		this.p('TargetAttributes = {', 4);
		this.p(targetId + ' = {', 5);
		this.p('CreatedOnToolsVersion = 6.1.1;', 6);
		if (targetOptions.developmentTeam) {
			this.p('DevelopmentTeam = ' + targetOptions.developmentTeam + ';', 6);
		}
		this.p('};', 5);
		this.p('};', 4);
		this.p('};', 3);
		this.p('buildConfigurationList = ' + projectBuildConfigListId + ' /* Build configuration list for PBXProject "' + project.getSafeName() + '" */;', 3);
		this.p('compatibilityVersion = "Xcode 3.2";', 3);
		this.p('developmentRegion = en;', 3);
		this.p('hasScannedForEncodings = 0;', 3);
		this.p('knownRegions = (', 3);
		this.p('en,', 4);
		this.p('Base,', 4);
		this.p(');', 3);
		this.p('mainGroup = ' + mainGroupId + ';', 3);
		this.p('productRefGroup = ' + productsGroupId + ' /* Products */;', 3);
		this.p('projectDirPath = "";', 3);
		this.p('projectRoot = "";', 3);
		this.p('targets = (', 3);
		this.p(targetId + ' /* ' + project.getSafeName() + ' */,', 4);
		this.p(');', 3);
		this.p('};', 2);
		this.p('/* End PBXProject section */');
		this.p();
		this.p('/* Begin PBXResourcesBuildPhase section */');
		this.p(resourcesBuildId + ' /* Resources */ = {', 2);
		this.p('isa = PBXResourcesBuildPhase;', 3);
		this.p('buildActionMask = 2147483647;', 3);
		this.p('files = (', 3);
		this.p(debugDirBuildId + ' /* Deployment in Resources */,', 4);
		this.p(iconBuildId + ' /* Images.xcassets in Resources */,', 4);
		this.p(');', 3);
		this.p('runOnlyForDeploymentPostprocessing = 0;', 3);
		this.p('};', 2);
		this.p('/* End PBXResourcesBuildPhase section */');
		this.p();
		this.p('/* Begin PBXSourcesBuildPhase section */');
		this.p(sourceBuildId + ' /* Sources */ = {', 2);
		this.p('isa = PBXSourcesBuildPhase;', 3);
		this.p('buildActionMask = 2147483647;', 3);
		this.p('files = (', 3);
		for (let file of files) {
			if (file.isBuildFile())
				this.p(file.getBuildId() + ' /* ' + file.toString() + ' in Sources */,', 4);
		}
		this.p(');', 3);
		this.p('runOnlyForDeploymentPostprocessing = 0;');
		this.p('};');
		this.p('/* End PBXSourcesBuildPhase section */');
		this.p();
		this.p('/* Begin XCBuildConfiguration section */');
		this.p(debugId + ' /* Debug */ = {', 2);
		this.p('isa = XCBuildConfiguration;', 3);
		this.p('buildSettings = {', 3);
		this.p('ALWAYS_SEARCH_USER_PATHS = NO;', 4);
		if (project.cppStd !== '' && project.cppStd !== 'gnu++14') {
			this.p('CLANG_CXX_LANGUAGE_STANDARD = "' + project.cppStd + '";', 4);
		}
		else {
			this.p('CLANG_CXX_LANGUAGE_STANDARD = "gnu++14";', 4);
		}
		this.p('CLANG_CXX_LIBRARY = "compiler-default";', 4);
		this.p('CLANG_ENABLE_MODULES = YES;', 4);
		this.p('CLANG_ENABLE_OBJC_ARC = YES;', 4);
		this.p('CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = YES;', 4);
		this.p('CLANG_WARN_BOOL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_COMMA = YES;', 4);
		this.p('CLANG_WARN_CONSTANT_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS = YES;', 4);
		this.p('CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;', 4);
		this.p('CLANG_WARN_EMPTY_BODY = YES;', 4);
		this.p('CLANG_WARN_ENUM_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_INFINITE_RECURSION = YES;', 4);
		this.p('CLANG_WARN_INT_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_NON_LITERAL_NULL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF = YES;', 4);
		this.p('CLANG_WARN_OBJC_LITERAL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;', 4);
		this.p('CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER = YES;', 4);
		this.p('CLANG_WARN_RANGE_LOOP_ANALYSIS = YES;', 4);
		this.p('CLANG_WARN_STRICT_PROTOTYPES = YES;', 4);
		this.p('CLANG_WARN_SUSPICIOUS_MOVE = YES;', 4);
		this.p('CLANG_WARN_UNREACHABLE_CODE = YES;', 4);
		this.p('CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;', 4);
		if (platform === 'ios') {
			this.p('"CODE_SIGN_IDENTITY[sdk=iphoneos*]" = "iPhone Developer";', 4);
		}
		else {
			this.p('CODE_SIGN_IDENTITY = "-";', 4);
		}
		this.p('COPY_PHASE_STRIP = NO;', 4);
		this.p('ENABLE_STRICT_OBJC_MSGSEND = YES;', 4);
		this.p('ENABLE_TESTABILITY = YES;', 4);
		if (project.cStd !== '' && project.cStd !== 'c99') {
			this.p('GCC_C_LANGUAGE_STANDARD = "' + project.cStd + '";', 4);
		}
		else {
			this.p('GCC_C_LANGUAGE_STANDARD = "gnu99";', 4);
		}
		this.p('GCC_DYNAMIC_NO_PIC = NO;', 4);
		this.p('GCC_NO_COMMON_BLOCKS = YES;', 4);
		this.p('GCC_OPTIMIZATION_LEVEL = 0;', 4);
		this.p('GCC_PREPROCESSOR_DEFINITIONS = (', 4);
		this.p('"DEBUG=1",', 5);
		for (const define of project.getDefines()) {
			if (define.config && define.config.toLowerCase() === 'release') {
				continue;
			}
			if (define.value.indexOf('=') >= 0)
				this.p('"' + define.value.replace(/\"/g, '\\\\\\"') + '",', 5);
			else
				this.p(define.value + ',', 5);
		}
		this.p('"$(inherited)",', 5);
		this.p(');', 4);
		this.p('GCC_SYMBOLS_PRIVATE_EXTERN = NO;', 4);
		this.p('GCC_WARN_64_TO_32_BIT_CONVERSION = YES;', 4);
		this.p('GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;', 4);
		this.p('GCC_WARN_UNDECLARED_SELECTOR = YES;', 4);
		this.p('GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;', 4);
		this.p('GCC_WARN_UNUSED_FUNCTION = YES;', 4);
		this.p('GCC_WARN_UNUSED_VARIABLE = YES;', 4);
		if (platform === 'ios') {
			this.p('IPHONEOS_DEPLOYMENT_TARGET = 11.0;', 4);
		}
		else {
			this.p('MACOSX_DEPLOYMENT_TARGET = 10.13;', 4);
		}
		this.p('MTL_ENABLE_DEBUG_INFO = YES;', 4);
		this.p('ONLY_ACTIVE_ARCH = YES;', 4);
		if (platform === 'ios') {
			this.p('SDKROOT = iphoneos;', 4);
			this.p('TARGETED_DEVICE_FAMILY = "1,2";', 4);
		}
		else {
			this.p('SDKROOT = macosx;', 4);
		}
		this.p('};', 3);
		this.p('name = Debug;', 3);
		this.p('};', 2);
		this.p(releaseId + ' /* Release */ = {', 2);
		this.p('isa = XCBuildConfiguration;', 3);
		this.p('buildSettings = {', 3);
		this.p('ALWAYS_SEARCH_USER_PATHS = NO;', 4);
		if (project.cppStd !== '' && project.cppStd !== 'gnu++14') {
			this.p('CLANG_CXX_LANGUAGE_STANDARD = "' + project.cppStd + '";', 4);
		}
		else {
			this.p('CLANG_CXX_LANGUAGE_STANDARD = "gnu++14";', 4);
		}
		this.p('CLANG_CXX_LIBRARY = "compiler-default";', 4);
		this.p('CLANG_ENABLE_MODULES = YES;', 4);
		this.p('CLANG_ENABLE_OBJC_ARC = YES;', 4);
		this.p('CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = YES;', 4);
		this.p('CLANG_WARN_BOOL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_COMMA = YES;', 4);
		this.p('CLANG_WARN_CONSTANT_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS = YES;', 4);
		this.p('CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;', 4);
		this.p('CLANG_WARN_EMPTY_BODY = YES;', 4);
		this.p('CLANG_WARN_ENUM_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_INFINITE_RECURSION = YES;', 4);
		this.p('CLANG_WARN_INT_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_NON_LITERAL_NULL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF = YES;', 4);
		this.p('CLANG_WARN_OBJC_LITERAL_CONVERSION = YES;', 4);
		this.p('CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;', 4);
		this.p('CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER = YES;', 4);
		this.p('CLANG_WARN_RANGE_LOOP_ANALYSIS = YES;', 4);
		this.p('CLANG_WARN_STRICT_PROTOTYPES = YES;', 4);
		this.p('CLANG_WARN_SUSPICIOUS_MOVE = YES;', 4);
		this.p('CLANG_WARN_UNREACHABLE_CODE = YES;', 4);
		this.p('CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;', 4);
		if (platform === 'ios') {
			this.p('"CODE_SIGN_IDENTITY[sdk=iphoneos*]" = "iPhone Developer";', 4);
		}
		else {
			this.p('CODE_SIGN_IDENTITY = "-";', 4);
		}
		this.p('COPY_PHASE_STRIP = YES;', 4);
		if (platform === 'osx') {
			this.p('DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";', 4);
		}
		this.p('ENABLE_NS_ASSERTIONS = NO;', 4);
		this.p('ENABLE_STRICT_OBJC_MSGSEND = YES;', 4);
		if (project.cStd !== '' && project.cStd !== 'c99') {
			this.p('GCC_C_LANGUAGE_STANDARD = "' + project.cStd + '";', 4);
		}
		else {
			this.p('GCC_C_LANGUAGE_STANDARD = "gnu99";', 4);
		}
		this.p('GCC_NO_COMMON_BLOCKS = YES;', 4);
		this.p('GCC_PREPROCESSOR_DEFINITIONS = (', 4);
		this.p('NDEBUG,', 5);
		for (const define of project.getDefines()) {
			if (define.config && define.config.toLowerCase() === 'debug') {
				continue;
			}
			if (define.value.indexOf('=') >= 0)
				this.p('"' + define.value.replace(/\"/g, '\\\\\\"') + '",', 5);
			else
				this.p(define.value + ',', 5);
		}
		this.p('"$(inherited)",', 5);
		this.p(');', 4);
		this.p('GCC_WARN_64_TO_32_BIT_CONVERSION = YES;', 4);
		this.p('GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;', 4);
		this.p('GCC_WARN_UNDECLARED_SELECTOR = YES;', 4);
		this.p('GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;', 4);
		this.p('GCC_WARN_UNUSED_FUNCTION = YES;', 4);
		this.p('GCC_WARN_UNUSED_VARIABLE = YES;', 4);
		if (platform === 'ios') {
			this.p('IPHONEOS_DEPLOYMENT_TARGET = 11.0;', 4);
		}
		else {
			this.p('MACOSX_DEPLOYMENT_TARGET = 10.13;', 4);
		}
		this.p('MTL_ENABLE_DEBUG_INFO = NO;', 4);
		if (platform === 'ios') {
			this.p('SDKROOT = iphoneos;', 4);
			this.p('TARGETED_DEVICE_FAMILY = "1,2";', 4);
			this.p('VALIDATE_PRODUCT = YES;', 4);
		}
		else {
			this.p('SDKROOT = macosx;', 4);
		}
		this.p('};', 3);
		this.p('name = Release;', 3);
		this.p('};', 2);
		this.p(nativeDebugId + ' /* Debug */ = {', 2);
		this.p('isa = XCBuildConfiguration;', 3);
		this.p('buildSettings = {', 3);
		this.p('ARCHS = arm64;', 4);
		this.p('ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;', 4);
		if (platform === 'osx') {
			this.p('COMBINE_HIDPI_IMAGES = YES;', 4);
		}
		this.p('FRAMEWORK_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		// Search paths to local frameworks
		for (let framework of frameworks) {
			if (framework.localPath != null)
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
		}
		this.p(');', 4);
		this.p('HEADER_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		this.p('"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include",', 5);
		for (let projectpath of project.getIncludeDirs())
			this.p('"' + path_resolve(from, projectpath).replace(/ /g, '\\\\ ') + '",', 5);
		this.p(');', 4);
		this.p('LIBRARY_SEARCH_PATHS = (', 4);
		for (let framework of frameworks) {
			if ((framework.toString().endsWith('.dylib') || framework.toString().endsWith('.a')) && framework.localPath != null) {
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
			}
		}
		this.p(');', 4);
		this.p('INFOPLIST_EXPAND_BUILD_SETTINGS = "YES";', 4);
		this.p('INFOPLIST_FILE = "' + path_resolve(from, plistname) + '";', 4);
		this.p('LD_RUNPATH_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		if (platform === 'ios') {
			this.p('"@executable_path/Frameworks",', 5);
		}
		for (let framework of frameworks) {
			if (framework.toString().endsWith('.dylib') && framework.localPath != null) {
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
			}
		}
		this.p(');', 4);
		if (project.cFlags.length > 0) {
			this.p('OTHER_CFLAGS = (', 4);
			for (let cFlag of project.cFlags) {
				this.p('"' + cFlag + '",', 5);
			}
			this.p(');', 4);
		}
		if (project.cppFlags.length > 0) {
			this.p('OTHER_CPLUSPLUSFLAGS = (', 4);
			for (let cppFlag of project.cppFlags) {
				this.p('"' + cppFlag + '",', 5);
			}
			this.p(');', 4);
		}
		this.p('PRODUCT_BUNDLE_IDENTIFIER = "' + targetOptions.bundle + '";', 4);
		this.p('BUNDLE_VERSION = "' + targetOptions.version + '";', 4);
		this.p('BUILD_VERSION = "' + targetOptions.build + '";', 4);
		this.p('CODE_SIGN_IDENTITY = "-";', 4);
		this.p('PRODUCT_NAME = "$(TARGET_NAME)";', 4);
		this.p('};', 3);
		this.p('name = Debug;', 3);
		this.p('};', 2);
		this.p(nativeReleaseId + ' /* Release */ = {', 2);
		this.p('isa = XCBuildConfiguration;', 3);
		this.p('buildSettings = {', 3);
		this.p('ARCHS = arm64;', 4);
		this.p('ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;', 4);
		if (platform === 'osx') {
			this.p('COMBINE_HIDPI_IMAGES = YES;', 4);
		}
		this.p('FRAMEWORK_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		// Search paths to local frameworks
		for (let framework of frameworks) {
			if (framework.localPath != null)
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
		}
		this.p(');', 4);
		this.p('HEADER_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		this.p('"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include",', 5);
		for (let p of project.getIncludeDirs())
			this.p('"' + path_resolve(from, p).replace(/ /g, '\\\\ ') + '",', 5);
		this.p(');', 4);
		this.p('LIBRARY_SEARCH_PATHS = (', 4);
		for (let framework of frameworks) {
			if ((framework.toString().endsWith('.dylib') || framework.toString().endsWith('.a')) && framework.localPath != null) {
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
			}
		}
		this.p(');', 4);
		this.p('INFOPLIST_EXPAND_BUILD_SETTINGS = "YES";', 4);
		this.p('INFOPLIST_FILE = "' + path_resolve(from, plistname) + '";', 4);
		this.p('LD_RUNPATH_SEARCH_PATHS = (', 4);
		this.p('"$(inherited)",', 5);
		if (platform === 'ios') {
			this.p('"@executable_path/Frameworks",', 5);
		}
		for (let framework of frameworks) {
			if (framework.toString().endsWith('.dylib') && framework.localPath != null) {
				this.p(framework.localPath.substr(0, framework.localPath.lastIndexOf('/')) + ',', 5);
			}
		}
		this.p(');', 4);
		if (project.cFlags.length > 0) {
			this.p('OTHER_CFLAGS = (', 4);
			for (let cFlag of project.cFlags) {
				this.p('"' + cFlag + '",', 5);
			}
			this.p(');', 4);
		}
		if (project.cppFlags.length > 0) {
			this.p('OTHER_CPLUSPLUSFLAGS = (', 4);
			for (let cppFlag of project.cppFlags) {
				this.p('"' + cppFlag + '",', 5);
			}
			this.p(');', 4);
		}
		this.p('PRODUCT_BUNDLE_IDENTIFIER = "' + targetOptions.bundle + '";', 4);
		this.p('BUNDLE_VERSION = "' + targetOptions.version + '";', 4);
		this.p('BUILD_VERSION = "' + targetOptions.build + '";', 4);
		this.p('CODE_SIGN_IDENTITY = "-";', 4);
		this.p('PRODUCT_NAME = "$(TARGET_NAME)";', 4);
		this.p('};', 3);
		this.p('name = Release;', 3);
		this.p('};', 2);
		this.p('/* End XCBuildConfiguration section */');
		this.p();
		this.p('/* Begin XCConfigurationList section */');
		this.p(projectBuildConfigListId + ' /* Build configuration list for PBXProject "' + project.getSafeName() + '" */ = {', 2);
		this.p('isa = XCConfigurationList;', 3);
		this.p('buildConfigurations = (', 3);
		this.p(debugId + ' /* Debug */,', 4);
		this.p(releaseId + ' /* Release */,', 4);
		this.p(');', 3);
		this.p('defaultConfigurationIsVisible = 0;', 3);
		this.p('defaultConfigurationName = Release;', 3);
		this.p('};', 2);
		this.p(nativeBuildConfigListId + ' /* Build configuration list for PBXNativeTarget "' + project.getSafeName() + '" */ = {', 2);
		this.p('isa = XCConfigurationList;', 3);
		this.p('buildConfigurations = (', 3);
		this.p(nativeDebugId + ' /* Debug */,', 4);
		this.p(nativeReleaseId + ' /* Release */,', 4);
		this.p(');', 3);
		this.p('defaultConfigurationIsVisible = 0;', 3);
		this.p('defaultConfigurationName = Release;', 3);
		this.p('};', 2);
		this.p('/* End XCConfigurationList section */');
		this.p('};', 1);
		this.p('rootObject = ' + projectId + ' /* Project object */;', 1);
		this.p('}');
		this.closeFile();
	}
}

class MakeExporter extends Exporter {
	constructor(options, cCompiler, cppCompiler, cFlags, cppFlags, linkerFlags, outputExtension, libsLine = null) {
		super(options);
		this.cCompiler = cCompiler;
		this.cppCompiler = cppCompiler;
		this.cFlags = cFlags;
		this.cppFlags = cppFlags;
		this.linkerFlags = linkerFlags;
		this.outputExtension = outputExtension;
		if (libsLine != null) {
			this.libsLine = libsLine;
		}
	}

	libsLine(project) {
		let libs = '';
		for (let lib of project.getLibs()) {
			libs += ' -l' + lib;
		}
		return libs;
	}

	exportSolution(project, from, to, platform, options) {
		let objects = {};
		let ofiles = {};
		let outputPath = path_resolve(to, options.buildPath);
		fs_ensuredir(outputPath);
		for (let fileobject of project.getFiles()) {
			let file = fileobject.file;
			if (file.endsWith('.cpp') || file.endsWith('.c') || file.endsWith('.cc')) {
				let name = file.toLowerCase();
				if (name.indexOf('/') >= 0)
					name = name.substr(name.lastIndexOf('/') + 1);
				name = name.substr(0, name.lastIndexOf('.'));
				if (!objects[name]) {
					objects[name] = true;
					ofiles[file] = name;
				}
				else {
					while (objects[name]) {
						name = name + '_';
					}
					objects[name] = true;
					ofiles[file] = name;
				}
			}
		}
		let ofilelist = '';
		for (let o in objects) {
			ofilelist += o + '.o ';
		}
		this.writeFile(path_resolve(outputPath, 'makefile'));
		let incline = '-I./ '; // local directory to pick up the precompiled headers
		for (let inc of project.getIncludeDirs()) {
			inc = path_relative(outputPath, path_resolve(from, inc));
			incline += '-I' + inc + ' ';
		}
		this.p('INC=' + incline);
		this.p('LIB=' + this.linkerFlags + this.libsLine(project));
		let defline = '';
		for (const def of project.getDefines()) {
			if (def.config && def.config.toLowerCase() === 'debug' && !options.debug) {
				continue;
			}
			if (def.config && def.config.toLowerCase() === 'release' && options.debug) {
				continue;
			}
			defline += '-D' + def.value.replace(/\"/g, '\\"') + ' ';
		}
		if (!options.debug) {
			defline += '-DNDEBUG ';
		}
		this.p('DEF=' + defline);
		this.p();
		let cline = this.cFlags;
		if (project.cStd !== '') {
			cline = '-std=' + project.cStd + ' ';
		}
		for (let flag of project.cFlags) {
			cline += flag + ' ';
		}
		this.p('CFLAGS=' + cline);
		let cppline = this.cppFlags;
		if (project.cppStd !== '') {
			cppline = '-std=' + project.cppStd + ' ';
		}
		for (let flag of project.cppFlags) {
			cppline += flag + ' ';
		}
		this.p('CPPFLAGS=' + cppline);
		let optimization = '';
		if (!options.debug) {
			optimization = '-O2';
		}
		else
			optimization = '-g';
		let executableName = project.getSafeName();
		if (project.getExecutableName()) {
			executableName = project.getExecutableName();
		}
		this.p(executableName + this.outputExtension + ': ' + ofilelist);
		let output = '-o "' + executableName + this.outputExtension + '"';
		this.p('\t' + this.cppCompiler + ' ' + output + ' ' + optimization + ' ' + ofilelist + ' $(LIB)');
		for (let fileobject of project.getFiles()) {
			let file = fileobject.file;
			if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('.cc')) {
				this.p();
				let name = ofiles[file];
				let realfile = path_relative(outputPath, path_resolve(from, file));
				this.p('-include ' + name + '.d');
				this.p(name + '.o: ' + realfile);
				let compiler = this.cppCompiler;
				let flags = '$(CPPFLAGS)';
				if (file.endsWith('.c')) {
					compiler = this.cCompiler;
					flags = '$(CFLAGS)';
				}
				this.p('\t' + compiler + ' ' + optimization + ' $(INC) $(DEF) -MD ' + flags + ' -c ' + realfile + ' -o ' + name + '.o');
			}
		}
		this.closeFile();
	}
}

class LinuxExporter extends Exporter {
	constructor(options) {
		super(options);
		let linkerFlags = '-static-libgcc -static-libstdc++ -pthread';
		let outputExtension = '';
		this.make = new MakeExporter(options, this.getCCompiler(), this.getCPPCompiler(), '', '', linkerFlags, outputExtension);
		this.compileCommands = new CompilerCommandsExporter(options);
	}

	exportSolution(project, from, to, platform, options) {
		this.make.exportSolution(project, from, to, platform, options);
		this.compileCommands.exportSolution(project, from, to, platform, options);
	}

	getCCompiler() {
		switch (Options_1.compiler) {
			case 'default':
			case 'clang':
				return 'clang';
			case 'gcc':
				return 'gcc';
		}
	}

	getCPPCompiler() {
		switch (Options_1.compiler) {
			case 'default':
			case 'clang':
				return 'clang++';
			case 'gcc':
				return 'g++';
		}
	}
}

class AndroidExporter extends Exporter {
	constructor(options) {
		super(options);
		this.compileCommands = new CompilerCommandsExporter(options);
	}
	exportSolution(project, from, to, platform, options) {
		this.safeName = project.getSafeName();
		const outdir = path_join(to.toString(), this.safeName);
		fs_ensuredir(outdir);
		const targetOptions = {
			package: 'tech.kinc',
			installLocation: 'internalOnly',
			versionCode: 1,
			versionName: '1.0',
			compileSdkVersion: 33,
			minSdkVersion: 24,
			targetSdkVersion: 33,
			screenOrientation: 'sensor',
			permissions: ['android.permission.VIBRATE'],
			disableStickyImmersiveMode: false,
			metadata: [],
			buildGradlePath: null,
			globalBuildGradlePath: null,
			proguardRulesPath: null,
			abiFilters: []
		};
		if (project.targetOptions != null && project.targetOptions.android != null) {
			const userOptions = project.targetOptions.android;
			for (let key in userOptions) {
				if (userOptions[key] == null)
					continue;
				switch (key) {
					case 'buildGradlePath':
					case 'globalBuildGradlePath':
					case 'proguardRulesPath':
						// fix path slashes and normalize
						const p = userOptions[key].split('/').join(path_sep);
						targetOptions[key] = path_join(from, p);
						break;
					default:
						targetOptions[key] = userOptions[key];
				}
			}
		}
		const binaryData = getEmbeddedBinaryData();
		const textData = getEmbeddedData();
		fs_writefile(path_join(outdir, '.gitignore'), textData['android_gitignore']);
		if (targetOptions.globalBuildGradlePath) {
			fs_copyfile(targetOptions.globalBuildGradlePath, path_join(outdir, 'build.gradle.kts'));
		}
		else {
			fs_writefile(path_join(outdir, 'build.gradle.kts'), textData['android_build_gradle']);
		}
		fs_writefile(path_join(outdir, 'gradle.properties'), textData['android_gradle_properties']);
		fs_writefile(path_join(outdir, 'gradlew'), textData['android_gradlew']);
		if (os_platform() !== 'win32') {
			os_chmod(path_join(outdir, 'gradlew'), 0o755);
		}
		fs_writefile(path_join(outdir, 'gradlew.bat'), textData['android_gradlew_bat']);
		let settings = textData['android_settings_gradle'];
		settings = settings.replace(/{name}/g, project.getName());
		fs_writefile(path_join(outdir, 'settings.gradle.kts'), settings);
		fs_ensuredir(path_join(outdir, 'app'));
		fs_writefile(path_join(outdir, 'app', '.gitignore'), textData['android_app_gitignore']);
		if (targetOptions.proguardRulesPath) {
			fs_copyfile(targetOptions.proguardRulesPath, path_join(outdir, 'app', 'proguard-rules.pro'));
		}
		else {
			fs_writefile(path_join(outdir, 'app', 'proguard-rules.pro'), textData['android_app_proguard_rules_pro']);
		}
		this.writeAppGradle(project, outdir, from, targetOptions, textData);
		this.writeCMakeLists(project, outdir, from, targetOptions, textData);
		fs_ensuredir(path_join(outdir, 'app', 'src'));
		fs_ensuredir(path_join(outdir, 'app', 'src', 'main'));
		this.writeManifest(outdir, targetOptions, textData);
		let strings = textData['android_main_res_values_strings_xml'];
		strings = strings.replace(/{name}/g, project.getName());
		fs_ensuredir(path_join(outdir, 'app', 'src', 'main', 'res', 'values'));
		fs_writefile(path_join(outdir, 'app', 'src', 'main', 'res', 'values', 'strings.xml'), strings);
		this.exportIcons(project.icon, outdir, from, to);
		fs_ensuredir(path_join(outdir, 'gradle', 'wrapper'));
		fs_writefile(path_join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.jar'), binaryData['android_gradle_wrapper_gradle_wrapper_jar']);
		fs_writefile(path_join(outdir, 'gradle', 'wrapper', 'gradle-wrapper.properties'), textData['android_gradle_wrapper_gradle_wrapper_properties']);
		if (project.getDebugDir().length > 0)
			fs_copydir(path_resolve(from, project.getDebugDir()), path_resolve(to, this.safeName, 'app', 'src', 'main', 'assets'));
		this.compileCommands.exportSolution(project, from, to, platform, options);
	}

	writeAppGradle(project, outdir, from, targetOptions, textData) {
		let cflags = '';
		for (let flag of project.cFlags)
			cflags += flag + ' ';
		let cppflags = '';
		for (let flag of project.cppFlags)
			cppflags += flag + ' ';
		let gradle = null;
		if (targetOptions.buildGradlePath) {
			gradle = fs_readfile(targetOptions.buildGradlePath);
		}
		else {
			gradle = textData['android_app_build_gradle'];
		}
		gradle = gradle.replace(/{package}/g, targetOptions.package);
		gradle = gradle.replace(/{versionCode}/g, targetOptions.versionCode.toString());
		gradle = gradle.replace(/{versionName}/g, targetOptions.versionName);
		gradle = gradle.replace(/{compileSdkVersion}/g, targetOptions.compileSdkVersion.toString());
		gradle = gradle.replace(/{minSdkVersion}/g, targetOptions.minSdkVersion.toString());
		gradle = gradle.replace(/{targetSdkVersion}/g, targetOptions.targetSdkVersion.toString());
		let arch = '';
		if (targetOptions.abiFilters.length > 0) {
			for (let item of targetOptions.abiFilters) {
				if (arch.length === 0) {
					arch = '"' + item + '"';
				}
				else {
					arch = arch + ', "' + item + '"';
				}
			}
			arch = `ndk { abiFilters += listOf(${arch}) }`;
		}
		else {
			switch (Options_1.arch) {
				case 'default':
					arch = '';
					break;
				case 'arm8':
					arch = 'arm64-v8a';
					break;
				case 'x86_64':
					arch = 'x86_64';
					break;
			}
			if (Options_1.arch !== 'default') {
				arch = `ndk {abiFilters += listOf("${arch}")}`;
			}
		}
		gradle = gradle.replace(/{architecture}/g, arch);
		gradle = gradle.replace(/{cflags}/g, cflags);
		cppflags = '-frtti -fexceptions ' + cppflags;
		if (project.cppStd !== '') {
			cppflags = '-std=' + project.cppStd + ' ' + cppflags;
		}
		gradle = gradle.replace(/{cppflags}/g, cppflags);
		let javasources = '';
		for (let dir of project.getJavaDirs()) {
			javasources += '"' + path_relative(path_join(outdir, 'app'), path_resolve(from, dir)).replace(/\\/g, '/') + '", ';
		}
		javasources += '"' + path_relative(path_join(outdir, 'app'), path_join(Project.kincDir.toString(), 'Backends', 'System', 'Android', 'Java-Sources')).replace(/\\/g, '/') + '"';
		gradle = gradle.replace(/{javasources}/g, javasources);
		fs_writefile(path_join(outdir, 'app', 'build.gradle.kts'), gradle);
	}

	writeCMakeLists(project, outdir, from, targetOptions, textData) {
		let cmake = textData['android_app_cmakelists_txt'];
		let debugDefines = '';
		for (const def of project.getDefines()) {
			if (!def.config || def.config.toLowerCase() === 'debug') {
				debugDefines += ' -D' + def.value.replace(/\"/g, '\\\\\\\"');
			}
		}
		cmake = cmake.replace(/{debug_defines}/g, debugDefines);
		let releaseDefines = '';
		for (const def of project.getDefines()) {
			if (!def.config || def.config.toLowerCase() === 'release') {
				releaseDefines += ' -D' + def.value.replace(/\"/g, '\\\\\\\"');
			}
		}
		cmake = cmake.replace(/{release_defines}/g, releaseDefines);
		let includes = '';
		for (let inc of project.getIncludeDirs()) {
			includes += '  "' + path_resolve(inc).replace(/\\/g, '/') + '"\n';
		}
		cmake = cmake.replace(/{includes}/g, includes);
		let files = '';
		for (let file of project.getFiles()) {
			if (file.file.endsWith('.c') || file.file.endsWith('.cc')
				|| file.file.endsWith('.cpp') || file.file.endsWith('.h')) {
				if (path_isabs(file.file)) {
					files += '  "' + path_resolve(file.file).replace(/\\/g, '/') + '"\n';
				}
				else {
					files += '  "' + path_resolve(path_join(from, file.file)).replace(/\\/g, '/') + '"\n';
				}
			}
		}
		cmake = cmake.replace(/{files}/g, files);
		let libraries1 = '';
		let libraries2 = '';
		for (let lib of project.getLibs()) {
			libraries1 += 'find_library(' + lib + '-lib ' + lib + ')\n';
			libraries2 += '  ${' + lib + '-lib}\n';
		}
		cmake = cmake.replace(/{libraries1}/g, libraries1)
			.replace(/{libraries2}/g, libraries2);
		const cmakePath = path_join(outdir, 'app', 'CMakeLists.txt');
		fs_writefile(cmakePath, cmake);
	}

	writeManifest(outdir, targetOptions, textData) {
		let manifest = textData['android_main_androidmanifest_xml'];
		manifest = manifest.replace(/{package}/g, targetOptions.package);
		manifest = manifest.replace(/{installLocation}/g, targetOptions.installLocation);
		manifest = manifest.replace(/{versionCode}/g, targetOptions.versionCode.toString());
		manifest = manifest.replace(/{versionName}/g, targetOptions.versionName);
		manifest = manifest.replace(/{screenOrientation}/g, targetOptions.screenOrientation);
		manifest = manifest.replace(/{targetSdkVersion}/g, targetOptions.targetSdkVersion);
		manifest = manifest.replace(/{permissions}/g, targetOptions.permissions.map((p) => { return '\n\t<uses-permission android:name="' + p + '"/>'; }).join(''));
		let metadata = targetOptions.disableStickyImmersiveMode ? '\n\t\t<meta-data android:name="disableStickyImmersiveMode" android:value="true"/>' : '';
		for (const meta of targetOptions.metadata) {
			metadata += '\n\t\t' + meta;
		}
		manifest = manifest.replace(/{metadata}/g, metadata);
		fs_ensuredir(path_join(outdir, 'app', 'src', 'main'));
		fs_writefile(path_join(outdir, 'app', 'src', 'main', 'AndroidManifest.xml'), manifest);
	}

	exportIcons(icon, outdir, from, to) {
		const folders = ['mipmap-mdpi', 'mipmap-hdpi', 'mipmap-xhdpi', 'mipmap-xxhdpi', 'mipmap-xxxhdpi'];
		const dpis = [48, 72, 96, 144, 192];
		for (let i = 0; i < dpis.length; ++i) {
			const folder = folders[i];
			const dpi = dpis[i];
			fs_ensuredir(path_join(outdir, 'app', 'src', 'main', 'res', folder));
			export_png(icon, path_resolve(to, this.safeName, 'app', 'src', 'main', 'res', folder, 'ic_launcher.png'), dpi, dpi, undefined, from);
			export_png(icon, path_resolve(to, this.safeName, 'app', 'src', 'main', 'res', folder, 'ic_launcher_round.png'), dpi, dpi, undefined, from);
		}
	}
}

class CompilerCommandsExporter extends Exporter {
	constructor(options) {
		super(options);
	}

	exportSolution(project, _from, to, platform, options) {
		let from = path_resolve(os_cwd(), _from);
		this.writeFile(path_resolve(to, 'compile_commands.json'));
		let includes = [];
		for (let inc of project.getIncludeDirs()) {
			includes.push('-I');
			includes.push(path_resolve(from, inc));
		}
		let defines = [];
		for (let def of project.getDefines()) {
			defines.push('-D');
			defines.push(def.value.replace(/\"/g, '\\"'));
		}
		let objects = {};
		let ofiles = {};
		for (let fileobject of project.getFiles()) {
			let file = fileobject.file;
			if (file.endsWith('.cpp') || file.endsWith('.c') || file.endsWith('.cc')) {
				let name = file.toLowerCase();
				if (name.indexOf('/') >= 0)
					name = name.substr(name.lastIndexOf('/') + 1);
				name = name.substr(0, name.lastIndexOf('.'));
				if (!objects[name]) {
					objects[name] = true;
					ofiles[file] = name;
				}
				else {
					while (objects[name]) {
						name = name + '_';
					}
					objects[name] = true;
					ofiles[file] = name;
				}
			}
		}

		let defaultArgs = [];
		if (platform === 'android') {
			defaultArgs.push('--target=aarch64-none-linux-android21');
			defaultArgs.push('-DANDROID');
			function ndkFromSdkRoot() {
				var _a;
				let sdkEnv = (_a = os_env('ANDROID_HOME')) !== null && _a !== void 0 ? _a : os_env('ANDROID_SDK_ROOT');
				if (!sdkEnv)
					return null;
				let ndk_dir = path_join(sdkEnv, 'ndk');
				if (!fs_exists(ndk_dir)) {
					return null;
				}
				let ndks = fs_readdir(ndk_dir);
				ndks = ndks.filter(item => !item.startsWith("."));
				if (ndks.length < 1) {
					return null;
				}
				return path_join(ndk_dir, ndks[0]);
			}
			let android_ndk = (_a = os_env('ANDROID_NDK')) !== null && _a !== void 0 ? _a : ndkFromSdkRoot();
			if (android_ndk) {
				let host_tag = '';
				switch (os_platform()) {
					case 'linux':
						host_tag = 'linux-x86_64';
						break;
					case 'darwin':
						host_tag = 'darwin-x86_64';
						break;
					case 'win32':
						host_tag = 'windows-x86_64';
						break;
				}
				let ndk_toolchain = path_join(android_ndk, `toolchains/llvm/prebuilt/${host_tag}`);
				if (host_tag !== '' && fs_exists(ndk_toolchain)) {
					defaultArgs.push(`--gcc-toolchain=${ndk_toolchain}`);
					defaultArgs.push(`--sysroot=${ndk_toolchain}/sysroot`);
				}
				else {
					// fallback to the first found toolchain
					let toolchains = fs_readdir(path_join(android_ndk, `toolchains/llvm/prebuilt/`));
					if (toolchains.length > 0) {
						let host_tag = toolchains[0];
						let ndk_toolchain = path_join(android_ndk, `toolchains/llvm/prebuilt/${host_tag}`);
						defaultArgs.push(`--gcc-toolchain=${ndk_toolchain}`);
						defaultArgs.push(`--sysroot=${ndk_toolchain}/sysroot`);
						console.log(`Found android ndk toolchain in ${ndk_toolchain}.`);
					}
				}
			}
		}

		let commands = [];
		for (let fileobject of project.getFiles()) {
			let file = fileobject.file;
			if (file.endsWith('.c') || file.endsWith('.cpp') || file.endsWith('.cc')) {
				let args = ['/usr/bin/clang', '-c', '-o', (options.debug ? 'Debug' : 'Release') + ofiles[file] + '.o'];
				if (file.endsWith('.c')) {
					args.push('-std=c99');
				}
				args.push(...defaultArgs);
				args.push(path_resolve(from, file));
				let command = {
					directory: from,
					file: path_resolve(from, file),
					output: path_resolve(to, ofiles[file] + '.o'),
					arguments: args.concat(includes).concat(defines)
				};
				commands.push(command);
			}
		}
		this.p(JSON.stringify(commands));
		this.closeFile();
	}
}

function exportKoremakeProject(from, to, platform, korefile, options) {
	console.log('Creating ' + platform + ' project files.');
	Project.root = path_resolve(from);

	let project = Project.create(from, to, platform, korefile);
	if (shaderLang(platform) === 'metal') {
		project.addFile(path_join(to, 'Sources', '*'), {});
	}
	project.resolveBackends();
	project.searchFiles(undefined);
	project.internalFlatten();
	fs_ensuredir(to);

	// Run again to find new shader files for Metal
	project.searchFiles(undefined);
	project.internalFlatten();

	let exporter = null;
	if (platform === 'ios' || platform === 'osx')
		exporter = new XCodeExporter(options);
	else if (platform === 'android')
		exporter = new AndroidExporter(options);
	else if (platform === 'wasm')
		exporter = new WasmExporter(options);
	else if (platform === 'linux')
		exporter = new LinuxExporter(options);
	else
		exporter = new VisualStudioExporter(options);

	exporter.exportSolution(project, from, to, platform, options);
	return project;
}

function compileProject(make, project, options) {
	if (make.status != 0) {
		os_exit(1);
	}
	let executableName = project.getSafeName();
	if (project.getExecutableName()) {
		executableName = project.getExecutableName();
	}
	if (options.target === 'linux') {
		let from = path_resolve(path_join(options.to.toString(), options.buildPath), executableName);
		let to = path_resolve(options.from.toString(), project.getDebugDir(), executableName);
		fs_copyfile(from, to);
		os_chmod(to, "+x");
	}
	else if (options.target === 'windows') {
		const extension = '.exe';
		const from = true
			? path_join(options.to.toString(), 'x64', options.debug ? 'Debug' : 'Release', executableName + extension)
			: path_join(options.to.toString(), options.debug ? 'Debug' : 'Release', executableName + extension);
		const dir = path_isabs(project.getDebugDir())
			? project.getDebugDir()
			: path_join(options.from.toString(), project.getDebugDir());
		fs_copyfile(from, path_join(dir, executableName + extension));
	}
	if (options.run) {
		if (options.target === 'osx') {
			os_exec('build/' + (options.debug ? 'Debug' : 'Release') + '/' + project.name + '.app/Contents/MacOS/' + project.name, [], { cwd: options.to });
		}
		else if (options.target === 'linux' || options.target === 'windows') {
			os_exec(path_resolve(options.from.toString(), project.getDebugDir(), executableName), [], { cwd: path_resolve(options.from.toString(), project.getDebugDir()) });
		}
	}
}

function main() {
	Project.kincDir = __dirname;
	Options_1.from = path_resolve(Options_1.from);
	Options_1.to = path_resolve(Options_1.to);
	console.log('Using Kinc from ' + Project.kincDir);
	Options_1.buildPath = Options_1.debug ? 'Debug' : 'Release';
	let project = exportKoremakeProject(Options_1.from, Options_1.to, Options_1.target, Options_1.kfile, Options_1);

	let solutionName = project.getSafeName();
	if (Options_1.compile && solutionName !== '') {
		console.log('Compiling...');
		let make = null;
		if (Options_1.target == 'linux' || Options_1.target == 'wasm') {
			let cores = os_cpus_length();
			make = os_exec('make', ['-j', cores.toString()], { cwd: path_join(Options_1.to, Options_1.buildPath) });
		}
		else if (Options_1.target == 'osx' || Options_1.target == 'ios') {
			let xcodeOptions = ['-configuration', Options_1.debug ? 'Debug' : 'Release', '-project', solutionName + '.xcodeproj'];
			if (Options_1.nosigning) {
				xcodeOptions.push('CODE_SIGN_IDENTITY=""');
				xcodeOptions.push('CODE_SIGNING_REQUIRED=NO');
				xcodeOptions.push('CODE_SIGNING_ALLOWED=NO');
			}
			make = os_exec('xcodebuild', xcodeOptions, { cwd: Options_1.to });
		}
		else if (Options_1.target == 'windows') {
			const vswhere = path_join(os_env('ProgramFiles(x86)'), 'Microsoft Visual Studio', 'Installer', 'vswhere.exe');
			let vsvars = os_exec(vswhere, ['-products', '*', '-latest', '-find', 'VC\\Auxiliary\\Build\\vcvars64.bat']).stdout.trim();
			fs_writefile(path_join(Options_1.to, 'build.bat'), '@call "' + vsvars + '"\n' + '@MSBuild.exe "' + path_resolve(Options_1.to, solutionName + '.vcxproj') + '" /m /clp:ErrorsOnly /p:Configuration=' + (Options_1.debug ? 'Debug' : 'Release') + ',Platform=x64');
			make = os_exec('build.bat', [], { cwd: Options_1.to });
		}
		else if (Options_1.target == 'android') {
			let gradlew = (os_platform() === 'win32') ? 'gradlew.bat' : 'bash';
			let args = (os_platform() === 'win32') ? [] : ['gradlew'];
			args.push('assemble' + (Options_1.debug ? 'Debug' : 'Release'));
			make = os_exec(gradlew, args, { cwd: path_join(Options_1.to, solutionName) });
		}
		if (make !== null) {
			compileProject(make, project, Options_1);
			return solutionName;
		}
	}
}

function default_target() {
	if (os_platform() === 'linux') {
		return 'linux';
	}
	else if (os_platform() === 'win32') {
		return 'windows';
	}
	else {
		return 'osx';
	}
}

let Options_1 = {
	from: '.',
	to: 'build',
	target: default_target(),
	graphics: 'default',
	visualstudio: 'vs2022',
	compile: false,
	run: false,
	debug: false,
	kfile: 'kfile.js',
	compiler: 'default',
	arch: 'default',
};

let args = scriptArgs;
for (let i = 1; i < args.length; ++i) {
	let arg = args[i];
	if (arg.startsWith("--")) {
		let name = arg.substring(2);
		let value = true;
		if (i < args.length - 1 && !args[i + 1].startsWith("--")) {
			++i;
			value = args[i];
		}
		Options_1[name] = value;
	}
}

if (Options_1.run) {
	Options_1.compile = true;
}

main();
console.log('Done.');
