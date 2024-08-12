
let project = new Project("amake");

project.addIncludeDir("../../sources/libs");
project.addFile("../../sources/libs/quickjs/*.c");
project.addFile("main.c");

if (platform === 'linux') {
	project.addLib("m");
	project.addDefine("_GNU_SOURCE");
	project.addDefine("environ=__environ");
	project.addDefine("sighandler_t=__sighandler_t");
}
else if (platform === "windows") {
	project.addDefine("WIN32_LEAN_AND_MEAN");
	project.addDefine("_WIN32_WINNT=0x0602");
	// C/C++ - Command Line:
	// /experimental:c11atomics /std:c11
	// Linker - Command Line:
	// /subsystem:console
}
else if (platform === "macos") {
	project.addFile("../../sources/backends/macos/kinc/backend/mac.plist");
}

// QuickJS changes:
// quickjs-libc.c#85 (fixes "import * as os from 'os';" crash):
// #define USE_WORKER -> //#define USE_WORKER
// "quickjs.h#259" (fixes "Maximum call stack size exceeded" in alang):
// #define JS_DEFAULT_STACK_SIZE (256 * 1024) -> #define JS_DEFAULT_STACK_SIZE (8 * 1024 * 1024)

return project;
