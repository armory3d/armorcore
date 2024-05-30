
let c_project = new Project(flags.name);
c_project.addProject('Kinc');
c_project.setDebugDir('Deployment');

if (fs_exists(os_cwd() + '/icon.png')) {
	c_project.icon = path_relative(__dirname, os_cwd()) + '/icon.png';
	if (platform === 'osx' && fs_exists(os_cwd() + '/icon_macos.png')) {
		c_project.icon = path_relative(__dirname, os_cwd()) + '/icon_macos.png';
	}
}

c_project.addIncludeDir('Sources/lib/gc');
c_project.addFile('Sources/lib/gc/*.c');
c_project.addIncludeDir('Sources');
c_project.addFile('Sources/krom.c');
let krom_c_path = path_relative(__dirname, os_cwd()) + '/build/krom.c';
c_project.addDefine('KROM_C_PATH="../' + krom_c_path + '"');

if (flags.with_audio) {
	c_project.addDefine('WITH_AUDIO');
}

if (flags.with_eval) {
	c_project.addDefine('WITH_EVAL');
}

if (flags.with_g2) {
	c_project.addDefine('WITH_G2');
	c_project.addFile('Sources/g2/*');
}

if (flags.with_iron) {
	c_project.addDefine('WITH_IRON');
	// c_project.addFile('Sources/iron/*.c');
	c_project.addFile('Sources/iron/iron_map.c');
	c_project.addFile('Sources/iron/iron_array.c');
	c_project.addFile('Sources/iron/iron_string.c');
	c_project.addFile('Sources/iron/iron_armpack.c');
	c_project.addFile('Sources/iron/iron_vec2.c');
	c_project.addFile('Sources/iron/iron_gc.c');
	c_project.addFile('Sources/iron/iron_json.c');
	c_project.addFile('Sources/iron/io_obj.c');
	c_project.addIncludeDir("Sources/lib/stb"); // iron_map.c -> stb_ds.h
	c_project.addIncludeDir('Sources/lib/jsmn'); // iron_json.c -> jsmn.h
}

if (flags.with_zui) {
	c_project.addDefine('WITH_ZUI');
	c_project.addFile('Sources/zui/*.c');
}

if (platform === 'windows') {
	c_project.addLib('Dbghelp'); // Stack walk
	c_project.addLib('Dwmapi'); // DWMWA_USE_IMMERSIVE_DARK_MODE
	if (flags.with_d3dcompiler && (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Direct3D12)) {
		c_project.addDefine('WITH_D3DCOMPILER');
		c_project.addLib("d3d11");
		c_project.addLib("d3dcompiler");
	}
}
else if (platform === 'linux') {
	c_project.addDefine("KINC_NO_WAYLAND"); // TODO: kinc_wayland_display_init() not implemented
}
else if (platform === 'osx') {

}
else if (platform === 'android') {
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'

	c_project.addFile('Sources/android/android_file_dialog.c');
	c_project.addFile('Sources/android/android_http_request.c');
	c_project.addDefine('IDLE_SLEEP');

	c_project.targetOptions.android.package = flags.package;
	c_project.targetOptions.android.permissions = ['android.permission.WRITE_EXTERNAL_STORAGE', 'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.INTERNET'];
	c_project.targetOptions.android.screenOrientation = ['sensorLandscape'];
	c_project.targetOptions.android.minSdkVersion = 30;
	c_project.targetOptions.android.targetSdkVersion = 33;
}
else if (platform === 'ios') {
	c_project.addFile('Sources/ios/ios_file_dialog.mm');
	c_project.addDefine('IDLE_SLEEP');
}

if (flags.with_nfd && (platform === 'windows' || platform === 'linux' || platform === 'osx')) {
	c_project.addDefine('WITH_NFD');
	c_project.addIncludeDir("Sources/lib/nfd");
	c_project.addFile('Sources/lib/nfd/nfd_common.c');

	if (platform === 'windows') {
		c_project.addFile('Sources/lib/nfd/nfd_win.cpp');
	}
	else if (platform === 'linux') {
		c_project.addFile('Sources/lib/nfd/nfd_gtk.c');
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
		c_project.addFile('Sources/lib/nfd/nfd_cocoa.m');
	}
}

if (flags.with_tinydir) {
	c_project.addDefine('WITH_TINYDIR');
	c_project.addIncludeDir("Sources/lib/tinydir");
}

if (flags.with_zlib) {
	c_project.addDefine('WITH_ZLIB');
	c_project.addIncludeDir("Sources/lib/zlib");
	c_project.addFile("Sources/lib/zlib/*.h");
	c_project.addFile("Sources/lib/zlib/*.c");
}

if (flags.with_stb_image_write) {
	c_project.addDefine('WITH_STB_IMAGE_WRITE');
	c_project.addIncludeDir("Sources/lib/stb");
}

if (flags.with_mpeg_write) {
	c_project.addDefine('WITH_MPEG_WRITE');
	c_project.addIncludeDir("Sources/lib/jo_mpeg");
}

if (flags.on_c_project_created) {
	flags.on_c_project_created(c_project);
}

c_project.flatten();
return c_project;
