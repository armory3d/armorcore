// ../../../Kinc/make --from ../../../ -g opengl --compiler clang --run

let raw: scene_t = {};

function start() {
	let ops: kinc_sys_ops_t = {};
	ops.title = "Empty";
	ops.width = 1280;
	ops.height = 720;
	ops.x = 0;
	ops.y = 0;
	ops.features = window_features_t.NONE;
	ops.mode = window_mode_t.WINDOWED;
	ops.frequency = 60;
	ops.vsync = true;
	sys_start(ops, sys_started);
}

function sys_started() {
	app_init(app_ready);
}

function render_commands() {
	render_path_set_target("");
	render_path_clear_target(0xff6495ed, 1.0, clear_flag_t.COLOR | clear_flag_t.DEPTH);
	render_path_draw_meshes("mesh");
}

function app_ready() {
	render_path_commands = render_commands;

	// raw = {};
	raw.name = "Scene";
	raw.shader_datas = [];
	raw.material_datas = [];
	raw.mesh_datas = [];
	raw.camera_datas = [];
	raw.camera_ref = "Camera";
	raw.objects = [];
	map_set(data_cached_scene_raws, raw.name, raw);

	let cd: camera_data_t = {};
	cd.name = "MyCamera";
	cd.near_plane = 0.1;
	cd.far_plane = 100.0;
	cd.fov = 0.85;
	array_push(raw.camera_datas, cd);

	let wvp_const: shader_const_t = {};
	wvp_const.name = "WVP";
	wvp_const.type = "mat4";
	wvp_const.link = "_world_view_proj_matrix";

	let tu: tex_unit_t = {};
	tu.name = "MyTexture";

	let pos: vertex_element_t = {};
	pos.name = "pos";
	pos.data = "short4norm";

	let tex: vertex_element_t = {};
	tex.name = "tex";
	tex.data = "short2norm";

	let sc: shader_context_t = {};
	sc.name = "mesh";
	sc.vertex_shader = "data/mesh.vert";
	sc.fragment_shader = "data/mesh.frag";
	sc.compare_mode = "less";
	sc.cull_mode = "clockwise";
	sc.depth_write = true;
	sc.constants = [wvp_const];
	sc.texture_units = [tu];
	sc.vertex_elements = [pos, tex];

	let sh: shader_data_t = {};
	sh.name = "MyShader";
	sh.contexts = [sc];
	array_push(raw.shader_datas, sh);

	let bt: bind_tex_t = {};
	bt.name = "MyTexture";
	bt.file = "texture.k";

	let mc: material_context_t = {};
	mc.name = "mesh";
	mc.bind_textures = [bt];

	let md: material_data_t = {};
	md.name = "MyMaterial";
	md.shader = "MyShader";
	md.contexts = [mc]
	array_push(raw.material_datas, md);

	material_data_parse(raw.name, md.name);
	data_ready();
}

function data_ready() {
	// Camera object
	let co: obj_t = {};
	co.name = "Camera";
	co.type = "camera_object";
	co.data_ref = "MyCamera";
	co.transform = null;
	array_push(raw.objects, co);

	// Mesh object
	let o: obj_t = {};
	o.name = "Cube";
	o.type = "mesh_object";
	o.data_ref = "cube.arm/Cube";
	o.material_refs = ["MyMaterial"];
	o.transform = null;
	array_push(raw.objects, o);

	// Instantiate scene
	scene_create(raw);

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
