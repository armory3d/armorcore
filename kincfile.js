
const release = process.argv.indexOf("--debug") == -1;
const with_d3dcompiler = true;
const with_nfd = true;
const with_tinydir = true;
const with_zlib = true;
const with_stb_image_write = false;
const with_audio = false;
let with_krafix = graphics === GraphicsApi.Vulkan; // glsl to spirv for vulkan

const system = platform === Platform.Windows ? "win32" :
			   platform === Platform.Linux   ? "linux" :
			   platform === Platform.OSX     ? "macos" :
			   platform === Platform.HTML5   ? "html5" :
			   platform === Platform.Android ? "android" :
			   platform === Platform.iOS     ? "ios" :
			   								   "unknown";

const build = release ? 'release' : 'debug';
const libdir = 'v8/libraries/' + system + '/' + build + '/';

let project = new Project('Krom');
project.cpp11 = true;
project.setDebugDir('Deployment');
project.addDefine('KINC_IMAGE_STANDARD_MALLOC');

if (with_audio) {
	project.addDefine('WITH_AUDIO');
}

if (platform === Platform.HTML5) {
	project.addFile('Sources/main_html5.c');
	// EmscriptenExporter.js:
	// -s EXTRA_EXPORTED_RUNTIME_METHODS=["cwrap"] -s ALLOW_TABLE_GROWTH
	// -s USE_WEBGL2=1 or -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2
}
else {
	project.addFile('Sources/main.cpp');
}

// Using newer V8 on Android and iOS, other platforms need to be updated
if (platform === Platform.Android) {
	project.addIncludeDir('v8/include_android');
	project.addDefine('IDLE_SLEEP');
}
else if (platform === Platform.iOS) {
	project.addFile('Sources/IOSFileDialog.mm');
	project.addIncludeDir('v8/include_ios');
	project.addDefine('IDLE_SLEEP');
}
else {
	project.addIncludeDir('v8/include');
}

if (platform === Platform.Windows) {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib(libdir + 'v8_monolith');
	if (with_d3dcompiler && (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Direct3D12)) {
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
	// project.addLib(libdir + 'libv8_monolith.a');

	// Some manual tweaking is required for now:
	// In Kinc/kincfile.js, before node Kinc/make is run:
	//   project.addDefine('KORE_ANDROID_API=18');
	// In app/CMakeLists.txt:
	//   add_library(v8_monolith STATIC IMPORTED)
	//   set_target_properties(v8_monolith PROPERTIES IMPORTED_LOCATION ../../../v8/libraries/android/release/libv8_monolith.a)
	//   target_link_libraries(v8_monolith)
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'
	// AndroidManifest.xml:
	//   <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
	//   <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
}
else if (platform === Platform.iOS) {
	project.addLib('libv8_monolith.a');
}
else if (platform === Platform.OSX) {
	project.addLib('libv8_monolith.a');
}

if (with_nfd && (platform === Platform.Windows || platform === Platform.Linux || platform === Platform.OSX)) {
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
if (with_zlib) {
	project.addDefine('WITH_ZLIB');
	project.addIncludeDir("Libraries/zlib");
	project.addFile("Libraries/zlib/*.h");
	project.addFile("Libraries/zlib/*.c");
	project.addExclude("Libraries/zlib/gzlib.c");
	project.addExclude("Libraries/zlib/gzclose.c");
	project.addExclude("Libraries/zlib/gzwrite.c");
	project.addExclude("Libraries/zlib/gzread.c");
}
if (with_stb_image_write) {
	project.addDefine('WITH_STB_IMAGE_WRITE');
	project.addIncludeDir("Libraries/stb");
	project.addFile("Libraries/stb/stb_image_write.h");
}
if (with_krafix) {
	// Set library to true in Libraries/krafix/kincfile.js
	await project.addProject('Libraries/krafix');
}

resolve(project);
