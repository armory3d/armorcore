
let project = new Project("minits");
project.addProject("../../Kinc");
project.setDebugDir("Deployment");

project.addIncludeDir("../../Sources");
project.addIncludeDir("../../Sources/lib/gc");
project.addIncludeDir("../../Sources/lib/stb");
project.addIncludeDir("../../Sources/lib/jsmn");

project.addFile("../../Sources/iron/*.c");
project.addFile("../../Sources/lib/gc/*.c");
project.addFile("test.c");
project.addDefine("NO_KROM_API");

project.flatten();
return project;
