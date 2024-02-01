"use strict";

(function () {

    let raw: TSceneFormat;

	function main() {
		System.start({
            title: "Empty",
            width: 1280,
            height: 720,
            x: 0,
            y: 0,
            features: WindowFeatures.FeatureNone,
            mode: WindowMode.Windowed,
            frequency: 60,
            vsync: true
        }, function() {
			App.init(app_ready);
		});
	}

	function app_ready() {
		RenderPath.commands = function() {
			RenderPath.setTarget("");
			RenderPath.clearTarget(0xff6495ed, 1.0);
			RenderPath.drawMeshes("mesh");
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
		Data.cachedSceneRaws.set(raw.name, raw);

		let cd: TCameraData = {
			name: "MyCamera",
			near_plane: 0.1,
			far_plane: 100.0,
			fov: 0.85
		};
		raw.camera_datas.push(cd);

		let sh: TShaderData = {
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
						{ name: "WVP", type: "mat4", link: "_worldViewProjectionMatrix" }
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

		let md: TMaterialData = {
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

		MaterialData.parse(raw.name, md.name, function(res: TMaterialData) {
			data_ready();
		});
	}

	function data_ready() {
		// Camera object
		let co: TObj = {
			name: "Camera",
			type: "camera_object",
			data_ref: "MyCamera",
			transform: null
		};
		raw.objects.push(co);

		// Mesh object
		let o: TObj = {
			name: "Cube",
			type: "mesh_object",
			data_ref: "cube.arm/Cube",
			material_refs: ["MyMaterial"],
			transform: null
		};
		raw.objects.push(o);

		// Instantiate scene
		Scene.create(raw, function(o: TBaseObject) {
			scene_ready();
		});
	}

	function scene_ready() {
		// Set camera
		let t = Scene.camera.transform;
		t.loc.set(0, -6, 0);
		t.rot.fromTo(Vec4.create(0, 0, 1), Vec4.create(0, -1, 0));
		t.buildMatrix();

		// Rotate cube
		let cube = Scene.getChild("Cube");
		App.notifyOnUpdate(function() {
			// cube.transform.rotate(Vec4.create(0, 0, 1), 0.02);
		});
	}

    main();
})();
