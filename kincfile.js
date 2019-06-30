const os = require('os');

let project = new Project('Krom');

const release = true;
const build = release ? 'release' : 'debug';
let system = 'linux';
if (os.platform() === 'darwin') {
	system = 'macos';
}
else if (os.platform() === 'win32') {
	system = 'win32';
}
const libdir = 'V8/Libraries/' + system + '/' + build + '/';

project.cpp11 = true;
project.addFile('Sources/**');
project.addIncludeDir('V8/include');

if (platform === Platform.Windows) {
	// if (!release) { //!
		project.addLib('Dbghelp');
		project.addLib('Shlwapi');
	// } //!
	project.addLib('bcrypt');
	project.addLib('Crypt32');
	project.addLib('Winmm');
	project.addLib(libdir + 'v8_monolith');
}

if (platform === Platform.OSX) {
	project.addLib(libdir + 'libv8_monolith.dylib');
}

if (platform === Platform.Linux) {
	project.addLib('../' + libdir + 'libv8_monolith.so');
	project.addLib('../' + libdir + 'libc++.so');
	project.addLib('libssl');
	project.addLib('libcrypto');
}

project.setDebugDir('Deployment/' + build + '/' + system);

resolve(project);

