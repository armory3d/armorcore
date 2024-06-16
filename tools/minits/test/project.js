
let project = new Project("minits");
project.addProject("../../Kinc");

project.addIncludeDir("../../sources");
project.addIncludeDir("../../sources/libs");

project.addFile("../../sources/iron/*.c");
project.addFile("../../sources/libs/gc.c");
project.addFile("test.c");
project.addDefine("NO_KROM_API");

project.flatten();
return project;
