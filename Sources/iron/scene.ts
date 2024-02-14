
let scene_camera: camera_object_t;
let scene_world: world_data_t;
let scene_meshes: mesh_object_t[];
let scene_lights: light_object_t[];
let scene_cameras: camera_object_t[];
///if arm_audio
let scene_speakers: speaker_object_t[];
///end
let scene_empties: object_t[];
let scene_animations: anim_raw_t[];
///if arm_skin
let scene_armatures: armature_t[];
///end
let scene_embedded: Map<string, image_t>;

let _scene_ready: bool;
let _scene_uid_counter = 0;
let _scene_uid: i32;
let _scene_raw: scene_t;
let _scene_root: object_t;
let _scene_scene_parent: object_t;
let _scene_objects_traversed: i32;
let _scene_objects_count: i32;

function scene_create(format: scene_t, done: (o: object_t)=>void) {
	_scene_uid = _scene_uid_counter++;
	scene_meshes = [];
	scene_lights = [];
	scene_cameras = [];
	///if arm_audio
	scene_speakers = [];
	///end
	scene_empties = [];
	scene_animations = [];
	///if arm_skin
	scene_armatures = [];
	///end
	scene_embedded = new Map();
	_scene_root = object_create();
	_scene_root.name = "Root";

	_scene_ready = false;
	_scene_raw = format;

	data_get_world(format.name, format.world_ref, function (world: world_data_t) {
		scene_world = world;

		// Startup scene
		scene_add_scene(format.name, null, function (scene_object: object_t) {
			if (scene_cameras.length == 0) {
				krom_log('No camera found for scene "' + format.name + '"');
			}

			scene_camera = scene_get_camera(format.camera_ref);
			_scene_scene_parent = scene_object;
			_scene_ready = true;
			done(scene_object);
		});
	});
}

function scene_remove() {
	for (let o of scene_meshes) {
		mesh_object_remove(o);
	}
	for (let o of scene_lights) {
		light_object_remove(o);
	}
	for (let o of scene_cameras) {
		camera_object_remove(o);
	}
	///if arm_audio
	for (let o of scene_speakers) {
		speaker_object_remove(o);
	}
	///end
	for (let o of scene_empties) {
		object_remove(o);
	}
	object_remove(_scene_root);
}

function scene_set_active(scene_name: string, done: (o: object_t)=>void = null) {
	if (_scene_root != null) {
		scene_remove();
	}

	data_get_scene_raw(scene_name, function (format: scene_t) {
		scene_create(format, function (o: object_t) {
			if (done != null) {
				done(o);
			}
			///if arm_voxels // Revoxelize
			_render_path_voxelized = 0;
			///end
		});
	});
}

function scene_update_frame() {
	if (!_scene_ready) {
		return;
	}
	for (let anim of scene_animations) {
		anim_update(anim, time_delta());
	}
	for (let e of scene_empties) {
		if (e != null && e.parent != null) {
			transform_update(e.transform);
		}
	}
}

function scene_render_frame() {
	if (!_scene_ready ||render_path_commands == null) {
		return;
	}

	// Render active camera
	scene_camera != null ? camera_object_render_frame(scene_camera) : render_path_render_frame();
}

// Objects
function scene_add_object(parent: object_t = null): object_t {
	let object = object_create();
	parent != null ? object_set_parent(object, parent) : object_set_parent(object, _scene_root);
	return object;
}

function scene_get_child(name: string): object_t {
	return object_get_child(_scene_root, name);
}

function scene_get_mesh(name: string): mesh_object_t {
	for (let m of scene_meshes) {
		if (m.base.name == name) {
			return m;
		}
	}
	return null;
}

function scene_get_light(name: string): light_object_t {
	for (let l of scene_lights) {
		if (l.base.name == name) {
			return l;
		}
	}
	return null;
}

function scene_get_camera(name: string): camera_object_t {
	for (let c of scene_cameras) {
		if (c.base.name == name) {
			return c;
		}
	}
	return null;
}

///if arm_audio
function scene_get_speaker(name: string): speaker_object_t {
	for (let s of scene_speakers) {
		if (s.base.name == name) {
			return s;
		}
	}
	return null;
}
///end

function scene_get_empty(name: string): object_t {
	for (let e of scene_empties) {
		if (e.name == name) {
			return e;
		}
	}
	return null;
}

function scene_add_mesh_object(data: mesh_data_t, materials: material_data_t[], parent: object_t = null): mesh_object_t {
	let object = mesh_object_create(data, materials);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}

function scene_add_light_object(data: light_data_t, parent: object_t = null): light_object_t {
	let object = light_object_create(data);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}

function scene_add_camera_object(data: camera_data_t, parent: object_t = null): camera_object_t {
	let object = camera_object_create(data);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}

///if arm_audio
function scene_add_speaker_object(data: speaker_data_t, parent: object_t = null): speaker_object_t {
	let object = speaker_object_create(data);
	parent != null ? object_set_parent(object.base, parent) : object_set_parent(object.base, _scene_root);
	return object;
}
///end

function scene_traverse_objects(format: scene_t, parent: object_t, objects: obj_t[], parent_object: obj_t, done: ()=>void) {
	if (objects == null) {
		return;
	}
	for (let i = 0; i < objects.length; ++i) {
		let o = objects[i];
		if (o.spawn != null && o.spawn == false) {
			if (++_scene_objects_traversed == _scene_objects_count) {
				done();
			}
			continue; // Do not auto-create Scene object
		}

		scene_create_object(o, format, parent, parent_object, function (object: object_t) {
			scene_traverse_objects(format, object, o.children, o, done);
			if (++_scene_objects_traversed == _scene_objects_count) {
				done();
			}
		});
	}
}

function scene_add_scene(scene_name: string, parent: object_t, done: (o: object_t)=>void) {
	if (parent == null) {
		parent = scene_add_object();
		parent.name = scene_name;
	}
	data_get_scene_raw(scene_name, function (format: scene_t) {
		scene_load_embedded_data(format.embedded_datas, function () { // Additional scene assets
			_scene_objects_traversed = 0;
			_scene_objects_count = scene_get_objects_count(format.objects);

			if (format.objects == null || format.objects.length == 0) {
				done(parent);
			}
			else {
				scene_traverse_objects(format, parent, format.objects, null, function () { // Scene objects
					done(parent);
				});
			}
		});
	});
}

function scene_get_objects_count(objects: obj_t[], discard_no_spawn = true): i32 {
	if (objects == null) {
		return 0;
	}
	let result = objects.length;
	for (let o of objects) {
		if (discard_no_spawn && o.spawn != null && o.spawn == false) {
			continue; // Do not count children of non-spawned objects
		}
		if (o.children != null) {
			result += scene_get_objects_count(o.children);
		}
	}
	return result;
}

function scene_spawn_object(name: string, parent: object_t, done: (o: object_t)=>void, spawn_children = true, src_raw: scene_t = null) {
	if (src_raw == null) {
		src_raw = _scene_raw;
	}
	let objects_traversed = 0;
	let obj = scene_get_raw_object_by_name(src_raw, name);
	let objects_count = spawn_children ? scene_get_objects_count([obj], false) : 1;
	let root_id = -1;
	function spawn_object_tree(obj: obj_t, parent: object_t, parent_object: obj_t, done: (o: object_t)=>void) {
		scene_create_object(obj, src_raw, parent, parent_object, function (object: object_t) {
			if (root_id == -1) {
				root_id = object.uid;
			}
			if (spawn_children && obj.children != null) {
				for (let child of obj.children) {
					spawn_object_tree(child, object, obj, done);
				}
			}
			if (++objects_traversed == objects_count && done != null) {
				// Retrieve the originally spawned object from the current
				// child object to ensure done() is called with the right
				// object
				while (object.uid != root_id) {
					object = object.parent;
				}
				done(object);
			}
		});
	}
	spawn_object_tree(obj, parent, null, done);
}

function scene_get_raw_object_by_name(format: scene_t, name: string): obj_t {
	return scene_traverse_objs(format.objects, name);
}

function scene_traverse_objs(children: obj_t[], name: string): obj_t {
	for (let o of children) {
		if (o.name == name) {
			return o;
		}
		if (o.children != null) {
			let res = scene_traverse_objs(o.children, name);
			if (res != null) {
				return res;
			}
		}
	}
	return null;
}

function scene_create_object(o: obj_t, format: scene_t, parent: object_t, parent_object: obj_t, done: (o: object_t)=>void) {
	let scene_name = format.name;

	if (o.type == "camera_object") {
		data_get_camera(scene_name, o.data_ref, function (b: camera_data_t) {
			let object = scene_add_camera_object(b, parent);
			scene_return_object(object.base, o, done);
		});
	}
	else if (o.type == "light_object") {
		data_get_light(scene_name, o.data_ref, function (b: light_data_t) {
			let object = scene_add_light_object(b, parent);
			scene_return_object(object.base, o, done);
		});
	}
	else if (o.type == "mesh_object") {
		if (o.material_refs == null || o.material_refs.length == 0) {
			scene_create_mesh_object(o, format, parent, parent_object, null, done);
		}
		else {
			// Materials
			let materials: material_data_t[] = [];
			let materials_loaded = 0;
			for (let i = 0; i < o.material_refs.length; ++i) {
				let ref = o.material_refs[i];
				data_get_material(scene_name, ref, function (mat: material_data_t) {
					materials[i] = mat;
					materials_loaded++;
					if (materials_loaded == o.material_refs.length) {
						scene_create_mesh_object(o, format, parent, parent_object, materials, done);
					}
				});
			}
		}
	}
	///if arm_audio
	else if (o.type == "speaker_object") {
		let object = scene_add_speaker_object(data_get_speaker_raw_by_name(format.speaker_datas, o.data_ref), parent);
		scene_return_object(object.base, o, done);
	}
	///end
	else if (o.type == "object") {
		let object = scene_add_object(parent);
		scene_return_object(object, o, function (ro: object_t) {
			done(ro);
		});
	}
	else {
		done(null);
	}
}

function scene_create_mesh_object(o: obj_t, format: scene_t, parent: object_t, parent_object: obj_t, materials: material_data_t[], done: (o: object_t)=>void) {
	// Mesh reference
	let ref = o.data_ref.split("/");
	let object_file = "";
	let data_ref = "";
	let scene_name = format.name;
	if (ref.length == 2) { // File reference
		object_file = ref[0];
		data_ref = ref[1];
	}
	else { // Local mesh data
		object_file = scene_name;
		data_ref = o.data_ref;
	}

	// Bone objects are stored in armature parent
	///if arm_skin
	if (parent_object != null && parent_object.bone_actions != null) {
		let bactions: scene_t[] = [];
		for (let ref of parent_object.bone_actions) {
			data_get_scene_raw(ref, function (action: scene_t) {
				bactions.push(action);
				if (bactions.length == parent_object.bone_actions.length) {
					let armature: armature_t = null;
					// Check if armature exists
					for (let a of scene_armatures) {
						if (a.uid == parent.uid) {
							armature = a;
							break;
						}
					}
					// Create new one
					if (armature == null) {
						// Unique name if armature was already instantiated for different object
						for (let a of scene_armatures) {
							if (a.name == parent.name) {
								parent.name += "." + parent.uid;
								break;
							}
						}
						armature = armature_create(parent.uid, parent.name, bactions);
						scene_armatures.push(armature);
					}
					scene_return_mesh_object(
						object_file, data_ref, scene_name, armature, materials, parent, parent_object, o, done);
				}
			});
		}
		return;
	}
	///end

	scene_return_mesh_object(object_file, data_ref, scene_name, null, materials, parent, parent_object, o, done);
}

function scene_return_mesh_object(object_file: string, data_ref: string, scene_name: string, armature: any, // armature_t
	materials: material_data_t[], parent: object_t, parentObject: obj_t, o: obj_t, done: (o: object_t)=>void) {

		data_get_mesh(object_file, data_ref, function (mesh: mesh_data_t) {
		///if arm_skin
		if (mesh.skin != null) {
			armature != null ? mesh_data_add_armature(mesh, armature) : mesh_data_add_action(mesh, _scene_raw.objects, "none");
		}
		///end
		let object = scene_add_mesh_object(mesh, materials, parent);

		// Attach particle systems
		///if arm_particles
		if (o.particle_refs != null) {
			for (let ref of o.particle_refs) {
				mesh_object_setup_particle_system(object, scene_name, ref);
			}
		}
		///end
		scene_return_object(object.base, o, done);
	});
}

function scene_return_object(object: object_t, o: obj_t, done: (o: object_t)=>void) {
	// Load object actions
	if (object != null && o.object_actions != null) {
		let oactions: scene_t[] = [];
		while (oactions.length < o.object_actions.length) {
			oactions.push(null);
		}
		let actions_loaded = 0;
		for (let i = 0; i < o.object_actions.length; ++i) {
			let ref = o.object_actions[i];
			if (ref == "null") { // No startup action set
				actions_loaded++;
				continue;
			}
			data_get_scene_raw(ref, function (action: scene_t) {
				oactions[i] = action;
				actions_loaded++;
				if (actions_loaded == o.object_actions.length) {
					scene_return_object_loaded(object, o, oactions, done);
				}
			});
		}
	}
	else scene_return_object_loaded(object, o, null, done);
}

function scene_return_object_loaded(object: object_t, o: obj_t, oactions: scene_t[], done: (o: object_t)=>void) {
	if (object != null) {
		object.raw = o;
		object.name = o.name;
		if (o.visible != null) {
			object.visible = o.visible;
		}
		scene_gen_transform(o, object.transform);
		object_setup_animation(object, oactions);
	}
	done(object);
}

function scene_gen_transform(object: obj_t, transform: transform_t) {
	transform.world = object.transform != null ? mat4_from_f32_array(object.transform.values) : mat4_identity();
	mat4_decompose(transform.world, transform.loc, transform.rot, transform.scale);
	// Whether to apply parent matrix
	if (object.local_only != null) {
		transform.local_only = object.local_only;
	}
	if (transform.object.parent != null) {
		transform_update(transform);
	}
}

function scene_load_embedded_data(datas: string[], done: ()=>void) {
	if (datas == null) {
		done();
		return;
	}
	let loaded = 0;
	for (let file of datas) {
		scene_embed_data(file, function () {
			loaded++;
			if (loaded == datas.length) {
				done();
			}
		});
	}
}

function scene_embed_data(file: string, done: ()=>void) {
	if (file.endsWith(".raw")) {
		data_get_blob(file, function (b: ArrayBuffer) {
			// Raw 3D texture bytes
			let w = Math.floor(Math.pow(b.byteLength, 1 / 3)) + 1;
			let image = image_from_bytes_3d(b, w, w, w, tex_format_t.R8);
			scene_embedded.set(file, image);
			done();
		});
	}
	else {
		data_get_image(file, function (image: image_t) {
			scene_embedded.set(file, image);
			done();
		});
	}
}