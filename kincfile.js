
const release = true;
const with_d3dcompiler = true;
const with_nfd = true;
const with_audio = false;

let project = new Project('Krom');
project.cpp11 = true;
project.addFile('Sources/*');
project.addIncludeDir('V8/include');
project.addDefine('KINC_IMAGE_STANDARD_MALLOC');

let system = platform === Platform.Windows ? "win32" :
			 platform === Platform.Linux ? "linux" : "macos";

const build = release ? 'release' : 'debug';
const libdir = 'V8/Libraries/' + system + '/' + build + '/';

if (platform === Platform.Windows) {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib(libdir + 'v8_monolith');
}
else if (platform === Platform.Linux) {
	project.addLib('v8_monolith -L../../' + libdir);
}
else if (platform === Platform.OSX) {
	project.addLib('libv8_monolith.a');
}

project.setDebugDir('Deployment');

if (with_d3dcompiler && platform === Platform.Windows) {
	project.addDefine('WITH_D3DCOMPILER');
	project.addLib("d3d11");
	project.addLib("d3dcompiler");
}

if (with_nfd) {
	project.addDefine('WITH_NFD');
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
}

if (with_audio) {
	project.addDefine('WITH_AUDIO');
}

resolve(project);
