
const release = true;
const with_d3dcompiler = true;
const with_nfd = true;
const with_tinydir = true;
const with_audio = false;

const system = platform === Platform.Windows ? "win32" :
			   platform === Platform.Linux ? "linux" :
			   platform === Platform.Android ? "android" :
			   platform === Platform.iOS ? "ios" : "macos";

const build = release ? 'release' : 'debug';
const libdir = 'v8/libraries/' + system + '/' + build + '/';

let project = new Project('Krom');
project.cpp11 = true;
project.setDebugDir('Deployment');
project.addFile('Sources/main.cpp');
project.addIncludeDir('v8/include');
project.addDefine('KINC_IMAGE_STANDARD_MALLOC');

if (platform === Platform.Windows) {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib(libdir + 'v8_monolith');
	if (with_d3dcompiler) {
		project.addDefine('WITH_D3DCOMPILER');
		project.addLib("d3d11");
		project.addLib("d3dcompiler");
	}
	if (!release) {
		project.addDefine('_HAS_ITERATOR_DEBUGGING=0');
		project.addDefine('_ITERATOR_DEBUG_LEVEL=0');
	}
}
else if (platform === Platform.Linux) {
	project.addLib('v8_monolith -L../../' + libdir);
}
else if (platform === Platform.Android) {
	project.addLib(libdir + 'libv8_monolith.a');
}
else if (platform === Platform.iOS) {
	project.addLib('libv8_monolith.a');
}
else if (platform === Platform.OSX) {
	project.addLib('libv8_monolith.a');
}

if (platform === Platform.Windows || platform === Platform.Linux || platform === Platform.OSX) {
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
	if (with_tinydir) {
		project.addDefine('WITH_TINYDIR');
		project.addIncludeDir("Libraries/tinydir/include");
	}
}

if (with_audio) {
	project.addDefine('WITH_AUDIO');
}

resolve(project);
