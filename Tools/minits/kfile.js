
let project = new Project("minits");
project.addProject("../../Kinc");
project.setDebugDir("Deployment");

project.addIncludeDir("../../Sources");
project.addIncludeDir("../../Libraries/gc");
project.addIncludeDir("../../Libraries/stb");

project.addFile("../../Sources/iron/*.c");
project.addFile("../../Libraries/gc/*.c");
project.addFile("test.c");
project.addDefine("NO_KROM_API");

project.flatten();
return project;
