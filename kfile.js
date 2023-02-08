const fs = require('fs');
let armorcore = false;

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
	await runKhamake();
	armorcore = true;

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
	with_audio: false,
	with_onnx: false,
	with_worker: false,
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

if (fs.existsSync("icon.png")) {
	project.icon = '../icon.png';
	if (platform === Platform.OSX && fs.existsSync("icon_macos.png")) {
		project.icon = '../icon_macos.png';
	}
}

if (flags.with_audio) {
	project.addDefine('WITH_AUDIO');
}

if (platform === Platform.Wasm) {
	project.addFile('Sources/main_wasm.c');
}
else {
	project.addFile('Sources/main.cpp');
	if (flags.with_worker) {
		project.addDefine('WITH_WORKER');
		project.addFile('Sources/worker.h');
		project.addFile('Sources/worker.cpp');
	}
}

project.addIncludeDir('v8/include');

if (platform === Platform.Android) {
	project.addFile('Sources/android/android_file_dialog.c');
	project.addFile('Sources/android/android_http_request.c');
	project.addDefine('IDLE_SLEEP');
	project.addJavaDir('Sources/android/java');

	project.targetOptions.android.package = flags.package;
	project.targetOptions.android.permissions = ['android.permission.WRITE_EXTERNAL_STORAGE', 'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.INTERNET'];
	project.targetOptions.android.screenOrientation = ['sensorLandscape'];
	project.targetOptions.android.minSdkVersion = 29;
	project.targetOptions.android.targetSdkVersion = 31;
}
else if (platform === Platform.iOS) {
	project.addFile('Sources/ios/ios_file_dialog.mm');
	project.addDefine('IDLE_SLEEP');
}

if (platform === Platform.Windows) {
	project.cmdArgs = [process.cwd() + '\\build\\krom'];
	project.addLib('Dbghelp'); // Stack walk
	project.addLib('Dwmapi'); // DWMWA_USE_IMMERSIVE_DARK_MODE
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
	//   set_target_properties(v8_monolith PROPERTIES IMPORTED_LOCATION ../../../../../../../armorcore/v8/libraries/android/release/libv8_monolith.a)
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
if (flags.with_onnx) {
	project.addDefine('WITH_ONNX');
	project.addIncludeDir("Libraries/onnx/include");
	if (platform === Platform.Windows) {
		project.addLib('Libraries/onnx/win32/onnxruntime');
	}
	else if (platform === Platform.Linux) {
		// patchelf --set-rpath . Armory
		if (armorcore) {
			project.addLib('onnxruntime -L../../armorcore/Libraries/onnx/linux');
		}
		else {
			project.addLib('onnxruntime -L../../Libraries/onnx/linux');
			//project.addLib('onnxruntime_providers_cuda -L../../Libraries/onnx/linux');
			//project.addLib('onnxruntime_providers_shared -L../../Libraries/onnx/linux');
		}
	}
	else if (platform === Platform.OSX) {
		project.addLib('Libraries/onnx/macos/libonnxruntime.1.13.1.dylib');
	}
}

if (flags.on_project_created) {
	flags.on_project_created(project);
}

project.flatten();
resolve(project);
