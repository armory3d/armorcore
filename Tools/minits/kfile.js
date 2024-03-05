
let project = new Project('minits');
await project.addProject('../../Kinc');
project.setDebugDir('Deployment');

project.addIncludeDir('../../Sources');
project.addIncludeDir('../../Libraries/gc');
project.addIncludeDir("../../Libraries/stb");

project.addFile('../../Sources/iron/*.c');
project.addFile('../../Libraries/gc/*.c');
project.addFile('test.c');

project.flatten();
resolve(project);
