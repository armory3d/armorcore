
let project = new Project("kmake");
project.setDebugDir("Deployment");

project.addIncludeDir("../../Sources/lib");
project.addFile("../../Sources/lib/quickjs/*.c");
project.addFile("main.c");

project.addLib("m");

project.addDefine("environ=__environ");
project.addDefine("sighandler_t=__sighandler_t");
// Undef "#define USE_WORKER" at "quickjs-libc.c#85" otherwise "import * as os from 'os';" crashes

project.flatten();
return project;
