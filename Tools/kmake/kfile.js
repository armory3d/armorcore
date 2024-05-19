
let project = new Project("kmake");
project.setDebugDir("Deployment");

project.addIncludeDir("../../Sources/lib");
project.addFile("../../Sources/lib/quickjs/*.c");
project.addFile("main.c");

project.addLib("m");

project.addDefine("environ=__environ");
project.addDefine("sighandler_t=__sighandler_t");

// quickjs-libc.c#85 (fixes "import * as os from 'os';" crash):
// #define USE_WORKER -> //#define USE_WORKER
// "quickjs.h#259" (fixes "Maximum call stack size exceeded" in minits):
// #define JS_DEFAULT_STACK_SIZE (256 * 1024) -> #define JS_DEFAULT_STACK_SIZE (8 * 1024 * 1024)

project.flatten();
return project;
