
///if arm_skin

class armature_t {
	uid: i32;
	name: string;
	actions: armature_action_t[] = [];
	mats_ready = false;
}

type armature_action_t = {
	name: string;
	bones: obj_t[];
	mats: mat4_t[];
}

function armature_create(uid: i32, name: string, actions: scene_t[]): armature_t {
	let raw = new armature_t();
	raw.uid = uid;
	raw.name = name;

	for (let a of actions) {
		for (let o of a.objects) {
			armature_set_parents(o);
		}
		let bones: obj_t[] = [];
		armature_traverse_bones(a.objects, function(object: obj_t) {
			bones.push(object);
		});
		raw.actions.push({ name: a.name, bones: bones, mats: null });
	}
	return raw;
}

function armature_init_mats(raw: armature_t) {
	if (raw.mats_ready) {
		return;
	}
	raw.mats_ready = true;

	for (let a of raw.actions) {
		if (a.mats != null) {
			continue;
		}
		a.mats = [];
		for (let b of a.bones) {
			a.mats.push(mat4_from_f32_array(b.transform.values));
		}
	}
}

function armature_get_action(raw: armature_t, name: string): armature_action_t {
	for (let a of raw.actions) {
		if (a.name == name) {
			return a;
		}
	}
	return null;
}

function armature_set_parents(object: obj_t) {
	if (object.children == null) {
		return;
	}
	for (let o of object.children) {
		o.parent = object;
		armature_set_parents(o);
	}
}

function armature_traverse_bones(objects: obj_t[], callback: (o: obj_t)=>void) {
	for (let i = 0; i < objects.length; ++i) {
		armature_traverse_bones_step(objects[i], callback);
	}
}

function armature_traverse_bones_step(object: obj_t, callback: (o: obj_t)=>void) {
	if (object.type == "bone_object") {
		callback(object);
	}
	if (object.children == null) {
		return;
	}
	for (let i = 0; i < object.children.length; ++i) {
		armature_traverse_bones_step(object.children[i], callback);
	}
}

///end
