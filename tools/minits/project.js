
let flags = globalThis.flags;

flags.on_c_project_created = function(c_project) {
    c_project.addDefine("NO_KROM_API");
	c_project.addDefine("NO_GC");

    c_project.addDefine("WITH_IRON");
	c_project.addFile("sources/iron_map.c");
	c_project.addFile("sources/iron_array.c");
	c_project.addFile("sources/iron_string.c");
	c_project.addFile("sources/iron_armpack.c");
	c_project.addFile("sources/iron_gc.c");
	c_project.addFile("sources/iron_json.c");
	c_project.addIncludeDir("sources/libs");
}

let project = new Project("minits");
project.addSources("./");
return project;
