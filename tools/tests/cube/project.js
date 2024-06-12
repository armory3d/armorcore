
let flags = globalThis.flags;
flags.with_iron = true;
flags.with_g2 = true;

let project = new Project("Test");
project.addSources("sources");
project.addShaders("shaders/*.glsl");
project.addShaders("../../../shaders/*.glsl");
project.addAssets("assets/*", { destination: "data/{name}" });

return project;
