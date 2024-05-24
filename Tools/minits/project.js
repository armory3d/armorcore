
let flags = globalThis.flags;

flags.on_c_project_created = function(c_project) {
    c_project.addDefine("NO_KROM_API");
	c_project.addDefine("NO_GC");

    c_project.addDefine('WITH_IRON');
	c_project.addFile('Sources/iron/iron_map.c');
	c_project.addFile('Sources/iron/iron_array.c');
	c_project.addFile('Sources/iron/iron_string.c');
	c_project.addFile('Sources/iron/iron_armpack.c');
	c_project.addFile('Sources/iron/iron_gc.c');
	c_project.addFile('Sources/iron/iron_json.c');
	c_project.addIncludeDir("Sources/lib/stb"); // iron_map.c -> stb_ds.h
	c_project.addIncludeDir('Sources/lib/jsmn'); // iron_json.c -> jsmn.h
}

let project = new Project("minits");
project.addSources("./");
return project;
