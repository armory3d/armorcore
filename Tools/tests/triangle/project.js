
let flags = globalThis.flags;
flags.with_iron = true;
flags.with_g2 = true;

let project = new Project("Test");
project.addSources("./");
return project;
