const fs = require('fs');
const path = require('path');

const project = new Project('Kinc');

project.addDefine('KINC_A1');
project.addDefine('KINC_A2');
project.addDefine('KINC_G1');
project.addDefine('KINC_G2');
let g4 = false;
let g5 = false;

project.addFile('Sources/**');
project.addIncludeDir('Sources');

project.addDefine('KINC_LZ4X');

function addBackend(name) {
	project.addFile('Backends/' + name + '/Sources/**');
	project.addIncludeDir('Backends/' + name + '/Sources');
}

if (platform === Platform.Windows) {
	addBackend('System/Windows');
	addBackend('System/Microsoft');
	project.addLib('dxguid');
	project.addLib('dsound');
	project.addLib('dinput8');
	project.addDefine('_CRT_SECURE_NO_WARNINGS');
	project.addDefine('_WINSOCK_DEPRECATED_NO_WARNINGS');
	project.addLib('ws2_32');
	project.addLib('Winhttp');
	project.addLib('wbemuuid');

	if (graphics === GraphicsApi.Direct3D11) {
		g4 = true;
		addBackend('Graphics4/Direct3D11');
		project.addDefine('KINC_DIRECT3D');
		project.addDefine('KINC_DIRECT3D11');
		project.addLib('d3d11');
	}
	else if (graphics === GraphicsApi.Direct3D12 || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Direct3D12');
		project.addDefine('KINC_DIRECT3D');
		project.addDefine('KINC_DIRECT3D12');
		project.addLib('dxgi');
		project.addLib('d3d12');
	}
	else if (graphics === GraphicsApi.Vulkan) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addDefine('KINC_VULKAN');
		project.addDefine('VK_USE_PLATFORM_WIN32_KHR');
		if (!process.env.VULKAN_SDK) {
			throw 'Could not find a Vulkan SDK';
		}
		project.addLib(path.join(process.env.VULKAN_SDK, 'Lib', 'vulkan-1'));
		let libs = fs.readdirSync(path.join(process.env.VULKAN_SDK, 'Lib'));
		for (const lib of libs) {
			if (lib.startsWith('VkLayer_')) {
				project.addLib(path.join(process.env.VULKAN_SDK, 'Lib', lib.substr(0, lib.length - 4)));
			}
		}
		project.addIncludeDir(path.join(process.env.VULKAN_SDK, 'Include'));
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Windows.');
	}

	if (audio === AudioApi.WASAPI || audio === AudioApi.Default) {
		addBackend('Audio2/WASAPI');
	}
	else {
		throw new Error('Audio API ' + audio + ' is not available for Windows.');
	}
}
else if (platform === Platform.OSX) {
	addBackend('System/Apple');
	addBackend('System/macOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		project.addDefine('KINC_METAL');
		project.addLib('Metal');
		project.addLib('MetalKit');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
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
else if (platform === Platform.iOS) {
	addBackend('System/Apple');
	addBackend('System/iOS');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Metal || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Metal');
		project.addDefine('KINC_METAL');
		project.addLib('Metal');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
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
else if (platform === Platform.Android) {
	project.addDefine('KINC_ANDROID');
	addBackend('System/Android');
	addBackend('System/POSIX');
	if (graphics === GraphicsApi.Vulkan || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addDefine('KINC_VULKAN');
		project.addDefine('VK_USE_PLATFORM_ANDROID_KHR');
		project.addLib('vulkan');
		project.addDefine('KINC_ANDROID_API=24');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
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
	if (Options.kong) {
		project.addLib('GLESv3');
	}
	else {
		project.addLib('GLESv2');
	}
	project.addLib('OpenSLES');
	project.addLib('OpenMAXAL');
}
else if (platform === Platform.Wasm) {
	project.addDefine('KINC_WASM');
	addBackend('System/Wasm');
	project.addIncludeDir('miniClib');
	project.addFile('miniClib/**');
	if (graphics === GraphicsApi.WebGPU) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/WebGPU');
		project.addDefine('KINC_WEBGPU');
	}
	else if (graphics === GraphicsApi.OpenGL || graphics === GraphicsApi.Default) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
		project.addDefine('KINC_OPENGL');
		project.addDefine('KINC_OPENGL_ES');
	}
	else {
		throw new Error('Graphics API ' + graphics + ' is not available for Wasm.');
	}
}
else if (platform === Platform.Linux) {
	addBackend('System/Linux');
	addBackend('System/POSIX');
	project.addLib('asound');
	project.addLib('dl');
	project.addLib('udev');

	try {
		if (!fs.existsSync(targetDirectory)) {
			fs.mkdirSync(targetDirectory);
		}
		if (!fs.existsSync(path.join(targetDirectory, 'wayland'))) {
			fs.mkdirSync(path.join(targetDirectory, 'wayland'));
		}
		const waylandDir = path.join(targetDirectory, 'wayland', 'wayland-generated');
		if (!fs.existsSync(waylandDir)) {
			fs.mkdirSync(waylandDir);
		}

		const child_process = require('child_process');

		let good_wayland = false;

		const wayland_call = child_process.spawnSync('wayland-scanner', ['--version'], {encoding: 'utf-8'});
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
			log.error('Could not parse wayland-version ' + wayland_version);
		}

		let c_ending = '.c';
		if (good_wayland) {
			c_ending = '.c.h';
		}

		let chfiles = [];

		function wl_protocol(protocol, file) {
			chfiles.push(file);
			const backend_path = path.resolve(waylandDir);
			const protocol_path = path.resolve('/usr/share/wayland-protocols', protocol);
			if (child_process.spawnSync('wayland-scanner', ['private-code', protocol_path, path.resolve(backend_path, file + c_ending)]).status !== 0) {
				throw 'Failed to generate wayland protocol files for' + protocol;
			}
			if (child_process.spawnSync('wayland-scanner', ['client-header', protocol_path, path.resolve(backend_path, file + '.h')]).status !== 0) {
				throw 'Failed to generate wayland protocol header for' + protocol;
			}
		}

		if (child_process.spawnSync('wayland-scanner', ['private-code', '/usr/share/wayland/wayland.xml', path.resolve(waylandDir, 'wayland-protocol' + c_ending)]).status !== 0) {
			throw 'Failed to generate wayland protocol files for /usr/share/wayland/wayland.xml';
		}
		if (child_process.spawnSync('wayland-scanner', ['client-header', '/usr/share/wayland/wayland.xml', path.resolve(waylandDir, 'wayland-protocol.h')]).status !== 0) {
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
			fs.writeFileSync(path.resolve(waylandDir, 'waylandunit.c'), cfile);
		}

		project.addIncludeDir(path.join(targetDirectory, 'wayland'));
		project.addFile(path.resolve(waylandDir, '**'));
	}
	catch (err) {
		log.error('Failed to include wayland-support, setting KINC_NO_WAYLAND.');
		log.error('Wayland error was: ' + err);
		project.addDefine('KINC_NO_WAYLAND');
	}

	if (graphics === GraphicsApi.Vulkan || graphics === GraphicsApi.Default) {
		g4 = true;
		g5 = true;
		addBackend('Graphics5/Vulkan');
		project.addLib('vulkan');
		project.addDefine('KINC_VULKAN');
	}
	else if (graphics === GraphicsApi.OpenGL) {
		g4 = true;
		addBackend('Graphics4/OpenGL');
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
	project.addExclude('Sources/kinc/graphics4/**');
}

if (g5) {
	project.addDefine('KINC_G5');
	project.addDefine('KINC_G4ONG5');
	addBackend('Graphics4/G4onG5');
}
else {
	project.addDefine('KINC_G5');
	project.addDefine('KINC_G5ONG4');
	addBackend('Graphics5/G5onG4');
}

project.setDebugDir('Deployment');
return project;
