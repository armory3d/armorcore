
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
project.addDefine('KINC_IMAGE_STANDARD_MALLOC');

if (platform === Platform.Android) {
	// Using newer V8 on Android, other platforms need to be updated
	project.addIncludeDir('v8/include_android');
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
	// In app/CMakeLists.txt:
	//   add_library(v8_monolith STATIC IMPORTED)
	//   set_target_properties(v8_monolith PROPERTIES IMPORTED_LOCATION ../../../v8/libraries/android/release/libv8_monolith.a)
	//   target_link_libraries(kore v8_monolith ...)
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'
	// In Kinc/kincfile.js:
	//   project.addDefine('KORE_ANDROID_API=18');
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

if (platform === Platform.Windows || platform === Platform.Linux || platform === Platform.OSX || platform === Platform.Android) {
	if (with_nfd && platform !== Platform.Android) {
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
