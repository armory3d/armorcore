// ../../../Kinc/make --from ../../../ -g opengl --compiler clang --run

let scene: scene_t;

function main() {
	let ops: kinc_sys_ops_t = {
		title: "Empty",
		width: 1280,
		height: 720,
		x: 0,
		y: 0,
		features: window_features_t.NONE,
		mode: window_mode_t.WINDOWED,
		frequency: 60,
		vsync: true
	};
	sys_start(ops);
	app_init();
	app_ready();
}

function render_commands() {
	render_path_set_target("");
	render_path_clear_target(0xff6495ed, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	render_path_draw_meshes("mesh");
}

function app_ready() {
	render_path_commands = render_commands;

	scene = {
		name: "Scene",
		objects: [],
		camera_datas: [],
		camera_ref: "Camera",
		material_datas: [],
		shader_datas: []
	};
	map_set(data_cached_scene_raws, scene.name, scene);

	let cd: camera_data_t = {
		name: "MyCamera",
		near_plane: 0.1,
		far_plane: 100.0,
		fov: 0.85,
	};
	array_push(scene.camera_datas, cd);

	let pos: vertex_element_t = {
		name: "pos",
		data: "short4norm"
	};

	let tex: vertex_element_t = {
		name: "tex",
		data: "short2norm"
	};

	let wvp: shader_const_t = {
		name: "WVP",
		type: "mat4",
		link: "_world_view_proj_matrix"
	};

	let tu: tex_unit_t = {
		name: "MyTexture"
	};

	let sc: shader_context_t = {
		name: "mesh",
		vertex_shader: "mesh.vert",
		fragment_shader: "mesh.frag",
		compare_mode: "less",
		cull_mode: "clockwise",
		depth_write: true
	};
	sc.vertex_elements = [pos, tex];
	sc.constants = [wvp];
	sc.texture_units = [tu];

	let sh: shader_data_t = {
		name: "MyShader"
	};
	sh.contexts = [sc];
	array_push(scene.shader_datas, sh);

	let bt: bind_tex_t = {
		name: "MyTexture",
		file: "texture.k"
	};

	let mc: material_context_t = {
		name: "mesh"
	};
	mc.bind_textures = [bt];

	let md: material_data_t = {
		name: "MyMaterial",
		shader: "MyShader"
	};
	md.contexts = [mc];
	array_push(scene.material_datas, md);

	material_data_parse(scene.name, md.name);
	data_ready();
}

function data_ready() {
	// Camera object
	let co: obj_t = {
		name: "Camera",
		type: "camera_object",
		data_ref: "MyCamera",
		visible: true,
		spawn: true
	};
	array_push(scene.objects, co);

	// Mesh object
	let o: obj_t = {
		name: "Cube",
		type: "mesh_object",
		data_ref: "cube.arm/Cube"
	};

	o.material_refs = ["MyMaterial"];
	o.visible = true;
	o.spawn = true;
	array_push(scene.objects, o);

	// Instantiate scene
	scene_create(scene);
	scene_ready();
}

function scene_ready() {
	// Set camera
	let t: transform_t = scene_camera.base.transform;
	vec4_set(t.loc, 0, -6, 0);
	quat_from_to(t.rot, vec4_create(0, 0, 1), vec4_create(0, -1, 0));
	transform_build_matrix(t);

	// Rotate cube
	app_notify_on_update(spin_cube);
}

function spin_cube() {
	let cube: object_t = scene_get_child("Cube");
	transform_rotate(cube.transform, vec4_create(0, 0, 1), 0.01);
}
