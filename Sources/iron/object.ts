
class object_t {
	uid: i32;
	urandom: f32;
	raw: obj_t;
	name: string;
	transform: transform_t;
	parent: object_t;
	children: object_t[];
	animation: anim_raw_t;
	visible: bool; // Skip render, keep updating
	culled: bool; // TBaseObject was culled last frame
	is_empty: bool;
	ext: any; // mesh_object_t | camera_object_t | light_object_t | speaker_object_t
}

let _object_uid_counter = 0;

function object_create(is_empty = true): object_t {
	let raw = new object_t();
	raw.name = "";
	raw.children = [];
	raw.visible = true;
	raw.culled = false;
	raw.uid = _object_uid_counter++;
	raw.transform = transform_create(raw);
	raw.is_empty = is_empty;
	if (raw.is_empty && _scene_ready) {
		scene_empties.push(raw);
	}
	return raw;
}

function object_set_parent(raw: object_t, parent_object: object_t, parent_inv = false, keep_transform = false) {
	if (parent_object == raw || parent_object == raw.parent) {
		return;
	}

	if (raw.parent != null) {
		array_remove(raw.parent.children, raw);
		if (keep_transform) {
			transform_apply_parent(raw.transform);
		}
		raw.parent = null; // rebuild matrix without a parent
		transform_build_matrix(raw.transform);
	}

	if (parent_object == null) {
		parent_object = _scene_scene_parent;
	}
	raw.parent = parent_object;
	raw.parent.children.push(raw);
	if (parent_inv) {
		transform_apply_parent_inv(raw.transform);
	}
}

function object_remove_super(raw: object_t) {
	if (raw.is_empty && _scene_ready) {
		array_remove(scene_empties, raw);
	}
	if (raw.animation != null) {
		anim_remove(raw.animation);
	}
	while (raw.children.length > 0) {
		object_remove(raw.children[0]);
	}
	if (raw.parent != null) {
		array_remove(raw.parent.children, raw);
		raw.parent = null;
	}
}

function object_remove(raw: object_t) {
	if (raw.ext != null)  {
		if (raw.ext.constructor == mesh_object_t) {
			mesh_object_remove(raw.ext);
		}
		else if (raw.ext.constructor == camera_object_t) {
			camera_object_remove(raw.ext);
		}
		else if (raw.ext.constructor == light_object_t) {
			light_object_remove(raw.ext);
		}
		///if arm_audio
		else if (raw.ext.constructor == speaker_object_t) {
			speaker_object_remove(raw.ext);
		}
		///end
	}
	else {
		object_remove_super(raw);
	}
}

function object_get_child(raw: object_t, name: string): object_t {
	if (raw.name == name) {
		return raw;
	}
	else {
		for (let c of raw.children) {
			let r = object_get_child(c, name);
			if (r != null) {
				return r;
			}
		}
	}
	return null;
}

function object_get_children(raw: object_t, recursive = false): object_t[] {
	if (!recursive) {
		return raw.children;
	}

	let ret_children = raw.children.slice();
	for (let child of raw.children) {
		ret_children = ret_children.concat(object_get_children(child, recursive));
	}
	return ret_children;
}

///if arm_skin
function object_get_parent_armature(raw: object_t, name: string): anim_bone_t {
	for (let a of scene_animations) {
		if (a.armature != null && a.armature.name == name) {
			return a.ext;
		}
	}
	return null;
}
///end

function object_setup_animation_super(raw: object_t, oactions: scene_t[] = null) {
	// Parented to bone
	///if arm_skin
	if (raw.raw.parent_bone != null) {
		app_notify_on_init(function () {
			let banim = object_get_parent_armature(raw, raw.parent.name);
			if (banim != null) {
				anim_bone_add_bone_child(banim, raw.raw.parent_bone, raw);
			}
		});
	}
	///end
	// TBaseObject actions
	if (oactions == null) {
		return;
	}
	raw.animation = anim_object_create(raw, oactions).base;
}

function object_setup_animation(raw: object_t, oactions: scene_t[] = null) {
	if (raw.ext != null)  {
		if (raw.ext.constructor == mesh_object_t) {
			mesh_object_setup_animation(raw.ext, oactions);
		}
	}
	else {
		object_setup_animation_super(raw, oactions);
	}
}
