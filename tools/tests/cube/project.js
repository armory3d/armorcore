
let flags = globalThis.flags;
flags.with_iron = true;
flags.with_g2 = true;
flags.with_ui = true;

let project = new Project("Test");
project.addProject("../../../");
project.addSources("sources");
project.addShaders("shaders/*.glsl");
project.addShaders("../../../shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });

return project;
