
let flags = globalThis.flags;
flags.with_minits = true;
flags.with_iron = true;
// flags.with_g2 = true;
// flags.with_zui = true;

let project = new Project("Test");
project.addSources("./");
resolve(project);
