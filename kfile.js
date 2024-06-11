
let project = new Project(flags.name);

{
	project.addDefine('KINC_A1');
	project.addDefine('KINC_A2');
	project.addDefine('KINC_G1');
	project.addDefine('KINC_G2');
	let g4 = false;
	let g5 = false;

	project.addFile('Sources/kinc/**');
	project.addIncludeDir('Sources');

	function addBackend(name) {
		project.addFile('Sources/backends/' + name + '/Sources/**');
		project.addIncludeDir('Sources/backends/' + name + '/Sources');
	}

	if (platform === 'windows') {
		addBackend('system/Windows');
		addBackend('system/Microsoft');
		project.addLib('dxguid');
		project.addLib('dsound');
		project.addLib('dinput8');
		project.addDefine('_CRT_SECURE_NO_WARNINGS');
		project.addDefine('_WINSOCK_DEPRECATED_NO_WARNINGS');
		project.addLib('ws2_32');
		project.addLib('Winhttp');
		project.addLib('wbemuuid');

		if (graphics === 'direct3d11') {
			g4 = true;
			addBackend('graphics4/Direct3D11');
			project.addDefine('KINC_DIRECT3D');
			project.addDefine('KINC_DIRECT3D11');
			project.addLib('d3d11');
		}
		else if (graphics === 'direct3d12' || graphics === 'default') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Direct3D12');
			project.addDefine('KINC_DIRECT3D');
			project.addDefine('KINC_DIRECT3D12');
			project.addLib('dxgi');
			project.addLib('d3d12');
		}
		else if (graphics === 'vulkan') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Vulkan');
			project.addDefine('KINC_VULKAN');
			project.addDefine('VK_USE_PLATFORM_WIN32_KHR');
			if (!os_env(VULKAN_SDK)) {
				throw 'Could not find a Vulkan SDK';
			}
			project.addLib(path_join(os_env(VULKAN_SDK), 'Lib', 'vulkan-1'));
			let libs = fs_readdir(path_join(os_env(VULKAN_SDK), 'Lib'));
			for (const lib of libs) {
				if (lib.startsWith('VkLayer_')) {
					project.addLib(path_join(os_env(VULKAN_SDK), 'Lib', lib.substr(0, lib.length - 4)));
				}
			}
			project.addIncludeDir(path_join(os_env(VULKAN_SDK), 'Include'));
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for Windows.');
		}

		addBackend('audio2/wasapi');
	}
	else if (platform === 'osx') {
		addBackend('system/Apple');
		addBackend('system/macOS');
		addBackend('system/POSIX');
		if (graphics === 'metal' || graphics === 'default') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Metal');
			project.addDefine('KINC_METAL');
			project.addLib('Metal');
			project.addLib('MetalKit');
		}
		else if (graphics === 'opengl') {
			g4 = true;
			addBackend('graphics4/OpenGL');
			project.addDefine('KINC_OPENGL');
			project.addLib('OpenGL');
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for macOS.');
		}
		project.addLib('IOKit');
		project.addLib('Cocoa');
		project.addLib('AppKit');
		project.addLib('CoreAudio');
		project.addLib('CoreData');
		project.addLib('CoreMedia');
		project.addLib('CoreVideo');
		project.addLib('AVFoundation');
		project.addLib('Foundation');
	}
	else if (platform === 'ios') {
		addBackend('system/Apple');
		addBackend('system/iOS');
		addBackend('system/POSIX');
		if (graphics === 'metal' || graphics === 'default') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Metal');
			project.addDefine('KINC_METAL');
			project.addLib('Metal');
		}
		else if (graphics === 'opengl') {
			g4 = true;
			addBackend('graphics4/OpenGL');
			project.addDefine('KINC_OPENGL');
			project.addDefine('KINC_OPENGL_ES');
			project.addLib('OpenGLES');
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for iOS.');
		}
		project.addLib('UIKit');
		project.addLib('Foundation');
		project.addLib('CoreGraphics');
		project.addLib('QuartzCore');
		project.addLib('CoreAudio');
		project.addLib('AudioToolbox');
		project.addLib('CoreMotion');
		project.addLib('AVFoundation');
		project.addLib('CoreFoundation');
		project.addLib('CoreVideo');
		project.addLib('CoreMedia');
	}
	else if (platform === 'android') {
		project.addDefine('KINC_ANDROID');
		addBackend('system/Android');
		addBackend('system/POSIX');
		if (graphics === 'vulkan' || graphics === 'default') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Vulkan');
			project.addDefine('KINC_VULKAN');
			project.addDefine('VK_USE_PLATFORM_ANDROID_KHR');
			project.addLib('vulkan');
			project.addDefine('KINC_ANDROID_API=24');
		}
		else if (graphics === 'opengl') {
			g4 = true;
			addBackend('graphics4/OpenGL');
			project.addDefine('KINC_OPENGL');
			project.addDefine('KINC_OPENGL_ES');
			project.addDefine('KINC_ANDROID_API=19');
			project.addDefine('KINC_EGL');
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for Android.');
		}
		project.addLib('log');
		project.addLib('android');
		project.addLib('EGL');
		project.addLib('GLESv3');
		project.addLib('OpenSLES');
		project.addLib('OpenMAXAL');
	}
	else if (platform === 'wasm') {
		project.addDefine('KINC_WASM');
		addBackend('system/Wasm');
		project.addIncludeDir('miniClib');
		project.addFile('miniClib/**');
		if (graphics === 'webgpu') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/WebGPU');
			project.addDefine('KINC_WEBGPU');
		}
		else if (graphics === 'opengl' || graphics === 'default') {
			g4 = true;
			addBackend('graphics4/OpenGL');
			project.addDefine('KINC_OPENGL');
			project.addDefine('KINC_OPENGL_ES');
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for Wasm.');
		}
	}
	else if (platform === 'linux') {
		addBackend('system/Linux');
		addBackend('system/POSIX');
		project.addLib('asound');
		project.addLib('dl');
		project.addLib('udev');

		try {
			if (!fs_exists(targetDirectory)) {
				fs_mkdir(targetDirectory);
			}
			if (!fs_exists(path_join(targetDirectory, 'wayland'))) {
				fs_mkdir(path_join(targetDirectory, 'wayland'));
			}
			const waylandDir = path_join(targetDirectory, 'wayland', 'wayland-generated');
			if (!fs_exists(waylandDir)) {
				fs_mkdir(waylandDir);
			}

			let good_wayland = false;

			const wayland_call = os_exec('wayland-scanner', ['--version']);
			if (wayland_call.status !== 0) {
				throw 'Could not run wayland-scanner to ask for its version';
			}
			const wayland_version = wayland_call.stderr;

			try {
				const scanner_versions = wayland_version.split(' ')[1].split('.');
				const w_x = parseInt(scanner_versions[0]);
				const w_y = parseInt(scanner_versions[1]);
				const w_z = parseInt(scanner_versions[2]);

				if (w_x > 1) {
					good_wayland = true;
				}
				else if (w_x === 1) {
					if (w_y > 17) {
						good_wayland = true;
					}
					else if (w_y === 17) {
						if (w_z >= 91) {
							good_wayland = true;
						}
					}
				}
			}
			catch (err) {
				console.log('Could not parse wayland-version ' + wayland_version);
			}

			let c_ending = '.c';
			if (good_wayland) {
				c_ending = '.c.h';
			}

			let chfiles = [];

			function wl_protocol(protocol, file) {
				chfiles.push(file);
				const backend_path = path_resolve(waylandDir);
				const protocol_path = path_resolve('/usr/share/wayland-protocols', protocol);
				if (os_exec('wayland-scanner', ['private-code', protocol_path, path_resolve(backend_path, file + c_ending)]).status !== 0) {
					throw 'Failed to generate wayland protocol files for' + protocol;
				}
				if (os_exec('wayland-scanner', ['client-header', protocol_path, path_resolve(backend_path, file + '.h')]).status !== 0) {
					throw 'Failed to generate wayland protocol header for' + protocol;
				}
			}

			if (os_exec('wayland-scanner', ['private-code', '/usr/share/wayland/wayland.xml', path_resolve(waylandDir, 'wayland-protocol' + c_ending)]).status !== 0) {
				throw 'Failed to generate wayland protocol files for /usr/share/wayland/wayland.xml';
			}
			if (os_exec('wayland-scanner', ['client-header', '/usr/share/wayland/wayland.xml', path_resolve(waylandDir, 'wayland-protocol.h')]).status !== 0) {
				throw 'Failed to generate wayland protocol header for /usr/share/wayland/wayland.xml';
			}
			wl_protocol('stable/viewporter/viewporter.xml', 'wayland-viewporter');
			wl_protocol('stable/xdg-shell/xdg-shell.xml', 'xdg-shell');
			wl_protocol('unstable/xdg-decoration/xdg-decoration-unstable-v1.xml', 'xdg-decoration');
			wl_protocol('unstable/tablet/tablet-unstable-v2.xml', 'wayland-tablet');
			wl_protocol('unstable/pointer-constraints/pointer-constraints-unstable-v1.xml', 'wayland-pointer-constraint');
			wl_protocol('unstable/relative-pointer/relative-pointer-unstable-v1.xml', 'wayland-relative-pointer');

			if (good_wayland) {
				let cfile = '#include "wayland-protocol.c.h"\n';
				for (const chfile of chfiles) {
					cfile += '#include "' + chfile + '.c.h"\n';
				}
				fs_writefile(path_resolve(waylandDir, 'waylandunit.c'), cfile);
			}

			project.addIncludeDir(path_join(targetDirectory, 'wayland'));
			project.addFile(path_resolve(waylandDir, '**'));
		}
		catch (err) {
			console.log('Failed to include wayland-support, setting KINC_NO_WAYLAND.');
			console.log('Wayland error was: ' + err);
			project.addDefine('KINC_NO_WAYLAND');
		}

		if (graphics === 'vulkan' || graphics === 'default') {
			g4 = true;
			g5 = true;
			addBackend('graphics5/Vulkan');
			project.addLib('vulkan');
			project.addDefine('KINC_VULKAN');
		}
		else if (graphics === 'opengl') {
			g4 = true;
			addBackend('graphics4/OpenGL');
			project.addLib('GL');
			project.addDefine('KINC_OPENGL');
			project.addLib('EGL');
			project.addDefine('KINC_EGL');
		}
		else {
			throw new Error('Graphics API ' + graphics + ' is not available for Linux.');
		}
		project.addDefine('_POSIX_C_SOURCE=200112L');
		project.addDefine('_XOPEN_SOURCE=600');
	}

	if (g4) {
		project.addDefine('KINC_G4');
	}
	else {
		project.addExclude('Kinc/Sources/kinc/graphics4/**');
	}

	if (g5) {
		project.addDefine('KINC_G5');
		project.addDefine('KINC_G4ONG5');
		addBackend('graphics4/G4onG5');
	}
	else {
		project.addDefine('KINC_G5');
		project.addDefine('KINC_G5ONG4');
		addBackend('graphics5/G5onG4');
	}
}

project.setDebugDir('Deployment');

if (fs_exists(os_cwd() + '/icon.png')) {
	project.icon = path_relative(__dirname, os_cwd()) + '/icon.png';
	if (platform === 'osx' && fs_exists(os_cwd() + '/icon_macos.png')) {
		project.icon = path_relative(__dirname, os_cwd()) + '/icon_macos.png';
	}
}

project.addIncludeDir('Sources/lib/gc');
project.addFile('Sources/lib/gc/*.c');
project.addIncludeDir('Sources');
project.addFile('Sources/krom.c');
let krom_c_path = path_relative(__dirname, os_cwd()) + '/build/krom.c';
project.addDefine('KROM_C_PATH="../' + krom_c_path + '"');

if (flags.with_audio) {
	project.addDefine('WITH_AUDIO');
}

if (flags.with_eval) {
	project.addDefine('WITH_EVAL');
}

if (flags.with_g2) {
	project.addDefine('WITH_G2');
}

if (flags.with_iron) {
	project.addDefine('WITH_IRON');
	// project.addFile('Sources/iron/*.c');
	project.addFile('Sources/iron/iron_map.c');
	project.addFile('Sources/iron/iron_array.c');
	project.addFile('Sources/iron/iron_string.c');
	project.addFile('Sources/iron/iron_armpack.c');
	project.addFile('Sources/iron/iron_vec2.c');
	project.addFile('Sources/iron/iron_gc.c');
	project.addFile('Sources/iron/iron_json.c');
	project.addFile('Sources/iron/io_obj.c');
	project.addIncludeDir("Sources/lib/stb"); // iron_map.c -> stb_ds.h
	project.addIncludeDir('Sources/lib/jsmn'); // iron_json.c -> jsmn.h
}

if (flags.with_zui) {
	project.addDefine('WITH_ZUI');
	project.addFile('Sources/zui/*.c');
}

if (platform === 'windows') {
	project.addLib('Dbghelp'); // Stack walk
	project.addLib('Dwmapi'); // DWMWA_USE_IMMERSIVE_DARK_MODE
	if (flags.with_d3dcompiler && (graphics === GraphicsApi.Direct3D11 || graphics === GraphicsApi.Direct3D12)) {
		project.addDefine('WITH_D3DCOMPILER');
		project.addLib("d3d11");
		project.addLib("d3dcompiler");
	}
}
else if (platform === 'linux') {
	project.addDefine("KINC_NO_WAYLAND"); // TODO: kinc_wayland_display_init() not implemented
}
else if (platform === 'osx') {

}
else if (platform === 'android') {
	// In app/build.gradle:
	//   android - defaultconfig - ndk.abiFilters 'arm64-v8a'
	project.addDefine('IDLE_SLEEP');
	project.targetOptions.android.package = flags.package;
	project.targetOptions.android.permissions = ['android.permission.WRITE_EXTERNAL_STORAGE', 'android.permission.READ_EXTERNAL_STORAGE', 'android.permission.INTERNET'];
	project.targetOptions.android.screenOrientation = ['sensorLandscape'];
	project.targetOptions.android.minSdkVersion = 30;
	project.targetOptions.android.targetSdkVersion = 33;
}
else if (platform === 'ios') {
	project.addDefine('IDLE_SLEEP');
}

if (flags.with_nfd && (platform === 'windows' || platform === 'linux' || platform === 'osx')) {
	project.addDefine('WITH_NFD');
	project.addIncludeDir("Sources/lib/nfd");
	project.addFile('Sources/lib/nfd/nfd_common.c');

	if (platform === 'windows') {
		project.addFile('Sources/lib/nfd/nfd_win.cpp');
	}
	else if (platform === 'linux') {
		project.addFile('Sources/lib/nfd/nfd_gtk.c');
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
		project.addFile('Sources/lib/nfd/nfd_cocoa.m');
	}
}

if (flags.with_tinydir) {
	project.addDefine('WITH_TINYDIR');
	project.addIncludeDir("Sources/lib/tinydir");
}

if (flags.with_zlib) {
	project.addDefine('WITH_ZLIB');
	project.addIncludeDir("Sources/lib/zlib");
	project.addFile("Sources/lib/zlib/*.h");
	project.addFile("Sources/lib/zlib/*.c");
}

if (flags.with_stb_image_write) {
	project.addDefine('WITH_STB_IMAGE_WRITE');
	project.addIncludeDir("Sources/lib/stb");
}

if (flags.with_mpeg_write) {
	project.addDefine('WITH_MPEG_WRITE');
	project.addIncludeDir("Sources/lib/jo_mpeg");
}

if (flags.on_c_project_created) {
	flags.on_c_project_created(project);
}

project.flatten();
return project;
