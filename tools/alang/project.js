
let project = new Project("alang");

flags.with_g2 = true;
flags.with_iron = true;
flags.with_zui = true;

project.addProject("../../");
project.addDefine("NO_GC");

project.addSources("./");
return project;
