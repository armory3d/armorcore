let project = new Project('to_spirv');

project.addDefine('KRAFIX_LIBRARY');
project.addFile('to_spirv.cpp');

project.addIncludeDir('glslang');
project.addFile('glslang/**');

return project;
