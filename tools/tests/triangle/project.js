
let flags = globalThis.flags;
flags.with_iron = true;
flags.with_g2 = true;
flags.with_ui = true;

let project = new Project("Test");
project.addProject("../../../");
project.addSources("./");
return project;
