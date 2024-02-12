// ../../../Kinc/make --from ../../../ -g opengl --compiler clang --run

(function () {

    let raw: scene_t;

	function main() {
		sys_start({
            title: "Empty",
            width: 1280,
            height: 720,
            x: 0,
            y: 0,
            features: window_features_t.NONE,
            mode: window_mode_t.WINDOWED,
            frequency: 60,
            vsync: true
        }, function() {
			app_init(app_ready);
		});
	}

	function app_ready() {
		render_path_commands = function() {
			render_path_set_target("");
			render_path_clear_target(0xff6495ed, 1.0);
			render_path_draw_meshes("mesh");
		};

		raw = {
			name: "Scene",
			shader_datas: [],
			material_datas: [],
			mesh_datas: [],
			camera_datas: [],
			camera_ref: "Camera",
			objects: []
		};
		data_cached_scene_raws.set(raw.name, raw);

		let cd: camera_data_t = {
			name: "MyCamera",
			near_plane: 0.1,
			far_plane: 100.0,
			fov: 0.85
		};
		raw.camera_datas.push(cd);

		let sh: shader_data_t = {
			name: "MyShader",
			contexts: [
				{
					name: "mesh",
					vertex_shader: "mesh.vert",
					fragment_shader: "mesh.frag",
					compare_mode: "less",
					cull_mode: "clockwise",
					depth_write: true,
					constants: [
						{ name: "WVP", type: "mat4", link: "_world_view_proj_matrix" }
					],
					texture_units: [
						{ name: "myTexture" }
					],
					vertex_elements: [
						{ name: "pos", data: "short4norm" },
						{ name: "tex", data: "short2norm" }
					]
				}
			]
		};
		raw.shader_datas.push(sh);

		let md: material_data_t = {
			name: "MyMaterial",
			shader: "MyShader",
			contexts: [
				{
					name: "mesh",
					bind_textures: [
						{ name: "myTexture", file: "texture.k" }
					]
				}
			]
		};
		raw.material_datas.push(md);

		material_data_parse(raw.name, md.name, function(res: material_data_t) {
			data_ready();
		});
	}

	function data_ready() {
		// Camera object
		let co: obj_t = {
			name: "Camera",
			type: "camera_object",
			data_ref: "MyCamera",
			transform: null
		};
		raw.objects.push(co);

		// Mesh object
		let o: obj_t = {
			name: "Cube",
			type: "mesh_object",
			data_ref: "cube.arm/Cube",
			material_refs: ["MyMaterial"],
			transform: null
		};
		raw.objects.push(o);

		// Instantiate scene
		scene_create(raw, function(o: object_t) {
			scene_ready();
		});
	}

	function scene_ready() {
		// Set camera
		let t = scene_camera.base.transform;
		vec4_set(t.loc, 0, -6, 0);
		quat_from_to(t.rot, vec4_create(0, 0, 1), vec4_create(0, -1, 0));
		transform_build_matrix(t);

		// Rotate cube
		let cube = scene_get_child("Cube");
		app_notify_on_update(function() {
			transform_rotate(cube.transform, vec4_create(0, 0, 1), 0.01);
		});
	}

    main();
})();
