
let flags = globalThis.flags;
flags.name = 'armorcore';
flags.package = 'org.armorcore';
flags.with_g2 = true;

let project = new Project(flags.name);
return project;
