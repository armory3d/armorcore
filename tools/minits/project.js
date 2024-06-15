
let flags = globalThis.flags;

let project = new Project("minits");

{
    project.addDefine("NO_KROM_API");
	project.addDefine("NO_GC");

    project.addDefine("WITH_IRON");
	project.addFile("sources/iron_map.c");
	project.addFile("sources/iron_array.c");
	project.addFile("sources/iron_string.c");
	project.addFile("sources/iron_armpack.c");
	project.addFile("sources/iron_gc.c");
	project.addFile("sources/iron_json.c");
	project.addIncludeDir("sources/libs");
}

project.addSources("./");
return project;
