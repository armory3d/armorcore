
let project = new Project("minits");
project.addProject("../../..");
project.addSources("./");
project.addDefine("NO_KROM_API");
project.flatten();
return project;
