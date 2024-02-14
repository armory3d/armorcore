
let project = new Project('minits');
await project.addProject('../../Kinc');
project.setDebugDir('Deployment');

project.addIncludeDir('../../Sources');
project.addFile('../../Sources/iron/*.c');
project.addFile('test.c');

project.flatten();
resolve(project);
