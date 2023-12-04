const fs = require('fs');
const path = require('path');

try {
	if (process.env.ARM_SNAPSHOT) {
		process.argv.push("--snapshot");
	}

	if (platform === Platform.Android || platform === Platform.Wasm) {
		process.argv.push("--shaderversion");
		process.argv.push("300");
	}

	let root = process.env.ARM_SDKPATH != undefined ? process.env.ARM_SDKPATH + "/armorcore" : __dirname;
	eval(fs.readFileSync(root + "/make.js") + "");
	await make_run();

	if (process.argv.indexOf("--run") >= 0) {
		fs.cp(process.cwd() + "/build/krom", __dirname + "/Deployment", {recursive: true}, function (err){});
	}

	if (process.env.ARM_HAXEONLY) {
		process.exit(1);
	}
}
catch (e) {
}

let flags = {
	name: 'Armory',
	package: 'org.armory3d',
	release: process.argv.indexOf("--debug") == -1,
	with_d3dcompiler: false,
	with_nfd: false,
	with_tinydir: false,
	with_zlib: false,
	with_stb_image_write: false,
	with_mpeg_write: false,
	with_audio: false,
	with_g2: false,
	with_iron: false,
	with_zui: false,
	on_project_created: null,
};

try {
	eval(fs.readFileSync("kincflags.js") + "");
}
catch (e) {
}

const system = platform === Platform.Windows ? "win32" :
			   platform === Platform.Linux   ? "linux" :
			   platform === Platform.OSX     ? "macos" :
			   platform === Platform.Wasm    ? "wasm" :
			   platform === Platform.Android ? "android" :
			   platform === Platform.iOS     ? "ios" :
			   								   "unknown";

const build = flags.release ? 'release' : 'debug';
let root = process.env.ARM_SDKPATH != undefined ? process.env.ARM_SDKPATH + "/armorcore" : __dirname;
const libdir = root + '/v8/libraries/' + system + '/' + build + '/';

let project = new Project(flags.name);
await project.addProject('Kinc');
project.cppStd = "c++17";
project.setDebugDir('Deployment');

if (fs.existsSync(process.cwd() + '/icon.png')) {
	project.icon = path.relative(__dirname, process.cwd()) + '/icon.png';
	if (platform === Platform.OSX && fs.existsSync(process.cwd() + '/icon_macos.png')) {
		project.icon = path.relative(__dirname, process.cwd()) + '/icon_macos.png';
	}
}

if (flags.with_audio) {
	project.addDefine('WITH_AUDIO');
}

if (platform === Platform.Wasm) {
	project.addFile('Sources/main_wasm.c');
	project.addFile('Shaders/**');
}
else {
	project.addFile('Sources/main.cpp');
}

if (flags.with_g2) {
	project.addDefine('WITH_G2');
	project.addFile('Sources/g2/*');
}

if (flags.with_iron) {
	project.addDefine('WITH_IRON');
	project.addFile('Sources/iron/*.c');
}

if (flags.with_zui) {
	project.addDefine('WITH_ZUI');
	project.addFile('Sources/zui/*.c');
}

project.addIncludeDir('v8/include');

if (platform === Platform.Android) {
	project.addFile('Sources/android/android_file_dialog.c');
	project.addFile('Sources/android/android_http_request.c');
	project.addDefine('IDLE_SLEEP');

	project.targetOptions.android.package = flags.package;
	project.targetOptions.android.permissions = ['android.permission.WRITE_EXTERNAL_STORAGE', 'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.INTERNET'];
	project.targetOptions.android.screenOrientation = ['sensorLandscape'];
	project.targetOptions.android.minSdkVersion = 30;
	project.targetOptions.android.targetSdkVersion = 33;
}
else if (platform === Platform.iOS) {
	project.addFile('Sources/ios/ios_file_dialog.mm');
	project.addDefine('IDLE_SLEEP');
}

if (platform === Platform.Windows) {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib('Dwmapi'); // DWMWA_USE_IMMERSIVE_DARK_MODE
	project.addLib('winmm'); // timeGetTime for V8
	project.addLib(libdir + 'v8_monolith');
	if (flags.with_d3dcompiler && (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Direct3D12)) {
		project.addDefine('WITH_D3DCOMPILER');
		project.addLib("d3d11");
		project.addLib("d3dcompiler");
	}
	if (!flags.release) {
		project.addDefine('_HAS_ITERATOR_DEBUGGING=0');
		project.addDefine('_ITERATOR_DEBUG_LEVEL=0');
	}
}
else if (platform === Platform.Linux) {
	project.addLib('v8_monolith -L' + libdir);
	project.addDefine("KINC_NO_WAYLAND"); // TODO: kinc_wayland_display_init() not implemented
}
else if (platform === Platform.Android) {
	// project.addLib(libdir + 'libv8_monolith.a');

	// Some manual tweaking is required for now:
	// In app/CMakeLists.txt:
	//   add_library(v8_monolith STATIC IMPORTED)
	//   set_target_properties(v8_monolith PROPERTIES IMPORTED_LOCATION ../../../../../../../../armorcore/v8/libraries/android/release/libv8_monolith.a)
	//   target_link_libraries(v8_monolith)
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'
}
else if (platform === Platform.iOS) {
	project.addLib('v8/libraries/ios/release/libv8_monolith.a');
}
else if (platform === Platform.OSX) {
	project.addLib('v8/libraries/macos/release/libv8_monolith.a');
}

if (flags.with_nfd && (platform === Platform.Windows || platform === Platform.Linux || platform === Platform.OSX)) {
	project.addDefine('WITH_NFD');
	project.addIncludeDir("Libraries/nfd/include");
	project.addFile('Libraries/nfd/nfd_common.c');

	if (platform === Platform.Windows) {
		project.addFile('Libraries/nfd/nfd_win.cpp');
	}
	else if (platform === Platform.Linux) {
		let gtk = true;
		if (gtk) {
			project.addFile('Libraries/nfd/nfd_gtk.c');

			project.addIncludeDir("/usr/include/gtk-3.0");
			project.addIncludeDir("/usr/include/glib-2.0");
			project.addIncludeDir("/usr/lib/x86_64-linux-gnu/glib-2.0/include");
			project.addIncludeDir("/usr/include/pango-1.0");
			project.addIncludeDir("/usr/include/cairo");
			project.addIncludeDir("/usr/include/gdk-pixbuf-2.0");
			project.addIncludeDir("/usr/include/atk-1.0");
			project.addIncludeDir("/usr/lib64/glib-2.0/include");
			project.addIncludeDir("/usr/lib/glib-2.0/include");
			project.addIncludeDir("/usr/include/harfbuzz");
			project.addLib('gtk-3');
			project.addLib('gobject-2.0');
			project.addLib('glib-2.0');
		}
		else {
			project.addFile('Libraries/nfd/nfd_zenity.c');
		}
	}
	else {
		project.addFile('Libraries/nfd/nfd_cocoa.m');
	}
}
if (flags.with_tinydir) {
	project.addDefine('WITH_TINYDIR');
	project.addIncludeDir("Libraries/tinydir/include");
}
if (flags.with_zlib) {
	project.addDefine('WITH_ZLIB');
	project.addIncludeDir("Libraries/zlib");
	project.addFile("Libraries/zlib/*.h");
	project.addFile("Libraries/zlib/*.c");
	project.addExclude("Libraries/zlib/gzlib.c");
	project.addExclude("Libraries/zlib/gzclose.c");
	project.addExclude("Libraries/zlib/gzwrite.c");
	project.addExclude("Libraries/zlib/gzread.c");
}
if (flags.with_stb_image_write) {
	project.addDefine('WITH_STB_IMAGE_WRITE');
	project.addIncludeDir("Libraries/stb");
}
if (flags.with_mpeg_write) {
	project.addDefine('WITH_MPEG_WRITE');
	project.addIncludeDir("Libraries/jo_mpeg");
}

if (flags.on_project_created) {
	await flags.on_project_created(project);
}

project.flatten();
resolve(project);
