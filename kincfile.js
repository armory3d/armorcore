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
	project.addLib("d3d11");
	project.addLib("d3dcompiler");
}

project.addIncludeDir("Libraries/nfd/include");
project.addFile('Libraries/nfd/nfd_common.c');

if (platform === Platform.Windows) {
	project.addFile('Libraries/nfd/nfd_win.cpp');
}
else if (platform === Platform.Linux) {
	project.addFile('Libraries/nfd/nfd_zenity.c');
}
else {
	project.addFile('Libraries/nfd/nfd_cocoa.m');
}

if (platform === Platform.Windows) {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib(libdir + 'v8_monolith');
}
else if (platform === Platform.Linux) {
	project.addLib('../' + libdir + 'libv8_monolith.so');
}
else if (platform === Platform.OSX) {
	project.addLib(libdir + 'libv8_monolith.a');
}

project.setDebugDir('Deployment');

resolve(project);
