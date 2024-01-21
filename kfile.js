const fs = require('fs');
const path = require('path');

globalThis.flags = {
	name: 'Armory',
	package: 'org.armory3d',
	dirname: __dirname,
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
	on_c_project_created: null,
};

try {
	if (process.env.ARM_SNAPSHOT) {
		process.argv.push("--snapshot");
	}

	if (platform === Platform.Android || platform === Platform.Wasm) {
		process.argv.push("--shaderversion");
		process.argv.push("300");
	}

	eval(fs.readFileSync(__dirname + "/make.js") + "");
	await make_run();

	if (process.argv.indexOf("--run") >= 0) {
		fs.cp(process.cwd() + "/build/krom", __dirname + "/Deployment", {recursive: true}, function (err){});
	}

	if (process.env.ARM_TSONLY) {
		process.exit(1);
	}
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
const libdir = __dirname + '/v8/libraries/' + system + '/' + build + '/';

let c_project = new Project(flags.name);
await c_project.addProject('Kinc');
c_project.cppStd = "c++17";
c_project.setDebugDir('Deployment');

if (fs.existsSync(process.cwd() + '/icon.png')) {
	c_project.icon = path.relative(__dirname, process.cwd()) + '/icon.png';
	if (platform === Platform.OSX && fs.existsSync(process.cwd() + '/icon_macos.png')) {
		c_project.icon = path.relative(__dirname, process.cwd()) + '/icon_macos.png';
	}
}

if (flags.with_audio) {
	c_project.addDefine('WITH_AUDIO');
}

if (platform === Platform.Wasm) {
	c_project.addFile('Sources/main_wasm.c');
	c_project.addFile('Shaders/**');
}
else {
	c_project.addFile('Sources/main.cpp');
}

if (flags.with_g2) {
	c_project.addDefine('WITH_G2');
	c_project.addFile('Sources/g2/*');
}

if (flags.with_iron) {
	c_project.addDefine('WITH_IRON');
	c_project.addFile('Sources/iron/*.c');
}

if (flags.with_zui) {
	c_project.addDefine('WITH_ZUI');
	c_project.addFile('Sources/zui/*.c');
}

c_project.addIncludeDir('v8/include');

if (platform === Platform.Android) {
	c_project.addFile('Sources/android/android_file_dialog.c');
	c_project.addFile('Sources/android/android_http_request.c');
	c_project.addDefine('IDLE_SLEEP');

	c_project.targetOptions.android.package = flags.package;
	c_project.targetOptions.android.permissions = ['android.permission.WRITE_EXTERNAL_STORAGE', 'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.INTERNET'];
	c_project.targetOptions.android.screenOrientation = ['sensorLandscape'];
	c_project.targetOptions.android.minSdkVersion = 30;
	c_project.targetOptions.android.targetSdkVersion = 33;
}
else if (platform === Platform.iOS) {
	c_project.addFile('Sources/ios/ios_file_dialog.mm');
	c_project.addDefine('IDLE_SLEEP');
}

if (platform === Platform.Windows) {
	c_project.addLib('Dbghelp'); // Stack walk
	c_project.addLib('Dwmapi'); // DWMWA_USE_IMMERSIVE_DARK_MODE
	c_project.addLib('winmm'); // timeGetTime for V8
	c_project.addLib(libdir + 'v8_monolith');
	if (flags.with_d3dcompiler && (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Direct3D12)) {
		c_project.addDefine('WITH_D3DCOMPILER');
		c_project.addLib("d3d11");
		c_project.addLib("d3dcompiler");
	}
	if (!flags.release) {
		c_project.addDefine('_HAS_ITERATOR_DEBUGGING=0');
		c_project.addDefine('_ITERATOR_DEBUG_LEVEL=0');
	}
}
else if (platform === Platform.Linux) {
	c_project.addLib('v8_monolith -L' + libdir);
	c_project.addDefine("KINC_NO_WAYLAND"); // TODO: kinc_wayland_display_init() not implemented
}
else if (platform === Platform.Android) {
	// c_project.addLib(libdir + 'libv8_monolith.a');
	// Some manual tweaking is required for now:
	// In app/CMakeLists.txt:
	//   add_library(v8_monolith STATIC IMPORTED)
	//   set_target_properties(v8_monolith PROPERTIES IMPORTED_LOCATION ../../../../../../../../armorcore/v8/libraries/android/release/libv8_monolith.a)
	//   target_link_libraries(v8_monolith)
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'
}
else if (platform === Platform.iOS) {
	c_project.addLib('v8/libraries/ios/release/libv8_monolith.a');
}
else if (platform === Platform.OSX) {
	c_project.addLib('v8/libraries/macos/release/libv8_monolith.a');
}

if (flags.with_nfd && (platform === Platform.Windows || platform === Platform.Linux || platform === Platform.OSX)) {
	c_project.addDefine('WITH_NFD');
	c_project.addIncludeDir("Libraries/nfd/include");
	c_project.addFile('Libraries/nfd/nfd_common.c');

	if (platform === Platform.Windows) {
		c_project.addFile('Libraries/nfd/nfd_win.cpp');
	}
	else if (platform === Platform.Linux) {
		c_project.addFile('Libraries/nfd/nfd_gtk.c');
		c_project.addIncludeDir("/usr/include/gtk-3.0");
		c_project.addIncludeDir("/usr/include/glib-2.0");
		c_project.addIncludeDir("/usr/lib/x86_64-linux-gnu/glib-2.0/include");
		c_project.addIncludeDir("/usr/include/pango-1.0");
		c_project.addIncludeDir("/usr/include/cairo");
		c_project.addIncludeDir("/usr/include/gdk-pixbuf-2.0");
		c_project.addIncludeDir("/usr/include/atk-1.0");
		c_project.addIncludeDir("/usr/lib64/glib-2.0/include");
		c_project.addIncludeDir("/usr/lib/glib-2.0/include");
		c_project.addIncludeDir("/usr/include/harfbuzz");
		c_project.addLib('gtk-3');
		c_project.addLib('gobject-2.0');
		c_project.addLib('glib-2.0');
	}
	else {
		c_project.addFile('Libraries/nfd/nfd_cocoa.m');
	}
}
if (flags.with_tinydir) {
	c_project.addDefine('WITH_TINYDIR');
	c_project.addIncludeDir("Libraries/tinydir/include");
}
if (flags.with_zlib) {
	c_project.addDefine('WITH_ZLIB');
	c_project.addIncludeDir("Libraries/zlib");
	c_project.addFile("Libraries/zlib/*.h");
	c_project.addFile("Libraries/zlib/*.c");
	c_project.addExclude("Libraries/zlib/gzlib.c");
	c_project.addExclude("Libraries/zlib/gzclose.c");
	c_project.addExclude("Libraries/zlib/gzwrite.c");
	c_project.addExclude("Libraries/zlib/gzread.c");
}
if (flags.with_stb_image_write) {
	c_project.addDefine('WITH_STB_IMAGE_WRITE');
	c_project.addIncludeDir("Libraries/stb");
}
if (flags.with_mpeg_write) {
	c_project.addDefine('WITH_MPEG_WRITE');
	c_project.addIncludeDir("Libraries/jo_mpeg");
}

if (flags.on_c_project_created) {
	await flags.on_c_project_created(c_project, platform, graphics);
}

c_project.flatten();
resolve(c_project);
