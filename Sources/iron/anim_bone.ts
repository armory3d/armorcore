
///if arm_skin

class anim_bone_t {
	base: anim_raw_t;
	object: mesh_object_t;
	data: mesh_data_t;
	skin_buffer: Float32Array;
	skeleton_bones: obj_t[] = null;
	skeleton_mats: mat4_t[] = null;
	skeleton_bones_blend: obj_t[] = null;
	skeleton_mats_blend: mat4_t[] = null;
	abs_mats: mat4_t[] = null;
	apply_parent: bool[] = null;
	mats_fast: mat4_t[] = [];
	mats_fast_sort: i32[] = [];
	mats_fast_blend: mat4_t[] = [];
	mats_fast_blend_sort: i32[] = [];
	bone_children: Map<string, object_t[]> = null; // Parented to bone
	// Do inverse kinematics here
	on_updates: (()=>void)[] = null;
}

let anim_bone_skin_max_bones = 128;
let anim_bone_m = mat4_identity(); // Skinning matrix
let anim_bone_m1 = mat4_identity();
let anim_bone_m2 = mat4_identity();
let anim_bone_bm = mat4_identity(); // Absolute bone matrix
let anim_bone_wm = mat4_identity();
let anim_bone_vpos = vec4_create();
let anim_bone_vscale = vec4_create();
let anim_bone_q1 = quat_create();
let anim_bone_q2 = quat_create();
let anim_bone_q3 = quat_create();
let anim_bone_vpos2 = vec4_create();
let anim_bone_vscale2 = vec4_create();
let anim_bone_v1 = vec4_create();
let anim_bone_v2 = vec4_create();

function anim_bone_create(armature_name = ""): anim_bone_t {
	let raw = new anim_bone_t();
	raw.base = anim_create();
	raw.base.ext = raw;

	raw.base.is_sampled = false;
	for (let a of scene_armatures) {
		if (a.name == armature_name) {
			raw.base.armature = a;
			break;
		}
	}
	return raw;
}

function anim_bone_get_num_bones(raw: anim_bone_t): i32 {
	if (raw.skeleton_bones == null) return 0;
	return raw.skeleton_bones.length;
}

function anim_bone_set_skin(raw: anim_bone_t, mo: mesh_object_t) {
	raw.object = mo;
	raw.data = mo != null ? mo.data : null;
	raw.base.is_skinned = raw.data != null ? raw.data.skin != null : false;
	if (raw.base.is_skinned) {
		let boneSize = 8; // Dual-quat skinning
		raw.skin_buffer = new Float32Array(anim_bone_skin_max_bones * boneSize);
		for (let i = 0; i < raw.skin_buffer.length; ++i) raw.skin_buffer[i] = 0;
		// Rotation is already applied to skin at export
		quat_set(raw.object.base.transform.rot, 0, 0, 0, 1);
		transform_build_matrix(raw.object.base.transform);

		let refs = mo.base.parent.raw.bone_actions;
		if (refs != null && refs.length > 0) {
			data_get_scene_raw(refs[0], (action: scene_t) => { anim_bone_play(raw, action.name); });
		}
	}
}

function anim_bone_add_bone_child(raw: anim_bone_t, bone: string, o: object_t) {
	if (raw.bone_children == null) raw.bone_children = new Map();
	let ar = raw.bone_children.get(bone);
	if (ar == null) {
		ar = [];
		raw.bone_children.set(bone, ar);
	}
	ar.push(o);
}

function anim_bone_remove_bone_child(raw: anim_bone_t, bone: string, o: object_t) {
	if (raw.bone_children != null) {
		let ar = raw.bone_children.get(bone);
		if (ar != null) array_remove(ar, o);
	}
}

function anim_bone_update_bone_children(raw: anim_bone_t, bone: obj_t, bm: mat4_t) {
	let ar = raw.bone_children.get(bone.name);
	if (ar == null) return;
	for (let o of ar) {
		let t = o.transform;
		if (t.bone_parent == null) t.bone_parent = mat4_identity();
		if (o.raw.parent_bone_tail != null) {
			if (o.raw.parent_bone_connected || raw.base.is_skinned) {
				let v = o.raw.parent_bone_tail;
				mat4_init_translate(t.bone_parent, v[0], v[1], v[2]);
				mat4_mult_mat(t.bone_parent, bm);
			}
			else {
				let v = o.raw.parent_bone_tail_pose;
				mat4_set_from(t.bone_parent, bm);
				mat4_translate(t.bone_parent, v[0], v[1], v[2]);
			}
		}
		else mat4_set_from(t.bone_parent, bm);
		transform_build_matrix(t);
	}
}

function anim_bone_num_parents(b: obj_t): i32 {
	let i = 0;
	let p = b.parent;
	while (p != null) {
		i++;
		p = p.parent;
	}
	return i;
}

function anim_bone_set_mats(raw: anim_bone_t) {
	while (raw.mats_fast.length < raw.skeleton_bones.length) {
		raw.mats_fast.push(mat4_identity());
		raw.mats_fast_sort.push(raw.mats_fast_sort.length);
	}
	// Calc bones with 0 parents first
	raw.mats_fast_sort.sort((a, b) => {
		let i = anim_bone_num_parents(raw.skeleton_bones[a]);
		let j = anim_bone_num_parents(raw.skeleton_bones[b]);
		return i < j ? -1 : i > j ? 1 : 0;
	});

	if (raw.skeleton_bones_blend != null) {
		while (raw.mats_fast_blend.length < raw.skeleton_bones_blend.length) {
			raw.mats_fast_blend.push(mat4_identity());
			raw.mats_fast_blend_sort.push(raw.mats_fast_blend_sort.length);
		}
		raw.mats_fast_blend_sort.sort((a, b) => {
			let i = anim_bone_num_parents(raw.skeleton_bones_blend[a]);
			let j = anim_bone_num_parents(raw.skeleton_bones_blend[b]);
			return i < j ? -1 : i > j ? 1 : 0;
		});
	}
}

function anim_bone_set_action(raw: anim_bone_t, action: string) {
	if (raw.base.is_skinned) {
		raw.skeleton_bones = raw.data._actions.get(action);
		raw.skeleton_mats = raw.data._mats.get(action);
		raw.skeleton_bones_blend = null;
		raw.skeleton_mats_blend = null;
	}
	else {
		armature_init_mats(raw.base.armature);
		let a = armature_get_action(raw.base.armature, action);
		raw.skeleton_bones = a.bones;
		raw.skeleton_mats = a.mats;
	}
	anim_bone_set_mats(raw);
}

function anim_bone_set_action_blend(raw: anim_bone_t, action: string) {
	if (raw.base.is_skinned) {
		raw.skeleton_bones_blend = raw.skeleton_bones;
		raw.skeleton_mats_blend = raw.skeleton_mats;
		raw.skeleton_bones = raw.data._actions.get(action);
		raw.skeleton_mats = raw.data._mats.get(action);
	}
	else {
		armature_init_mats(raw.base.armature);
		let a = armature_get_action(raw.base.armature, action);
		raw.skeleton_bones = a.bones;
		raw.skeleton_mats = a.mats;
	}
	anim_bone_set_mats(raw);
}

function anim_bone_mult_parent(raw: anim_bone_t, i: i32, fasts: mat4_t[], bones: obj_t[], mats: mat4_t[]) {
	let f = fasts[i];
	if (raw.apply_parent != null && !raw.apply_parent[i]) {
		mat4_set_from(f, mats[i]);
		return;
	}
	let p = bones[i].parent;
	let bi = anim_bone_get_bone_index(raw, p, bones);
	(p == null || bi == -1) ? mat4_set_from(f, mats[i]) : mat4_mult_mats(f, fasts[bi], mats[i]);
}

function anim_bone_mult_parents(raw: anim_bone_t, m: mat4_t, i: i32, bones: obj_t[], mats: mat4_t[]) {
	let bone = bones[i];
	let p = bone.parent;
	while (p != null) {
		let i = anim_bone_get_bone_index(raw, p, bones);
		if (i == -1) continue;
		mat4_mult_mat(m, mats[i]);
		p = p.parent;
	}
}

function anim_bone_notify_on_update(raw: anim_bone_t, f: ()=>void) {
	if (raw.on_updates == null) raw.on_updates = [];
	raw.on_updates.push(f);
}

function anim_bone_remove_update(raw: anim_bone_t, f: ()=>void) {
	array_remove(raw.on_updates, f);
}

function anim_bone_update_bones_only(raw: anim_bone_t) {
	if (raw.bone_children != null) {
		for (let i = 0; i < raw.skeleton_bones.length; ++i) {
			let b = raw.skeleton_bones[i]; // TODO: blendTime > 0
			mat4_set_from(anim_bone_m, raw.mats_fast[i]);
			anim_bone_update_bone_children(raw, b, anim_bone_m);
		}
	}
}

function anim_bone_update_skin_gpu(raw: anim_bone_t) {
	let bones = raw.skeleton_bones;

	let s: f32 = raw.base.blend_current / raw.base.blend_time;
	s = s * s * (3.0 - 2.0 * s); // Smoothstep
	if (raw.base.blend_factor != 0.0) s = 1.0 - raw.base.blend_factor;

	// Update skin buffer
	for (let i = 0; i < bones.length; ++i) {
		mat4_set_from(anim_bone_m, raw.mats_fast[i]);

		if (raw.base.blend_time > 0 && raw.skeleton_bones_blend != null) {
			// Decompose
			mat4_set_from(anim_bone_m1, raw.mats_fast_blend[i]);
			mat4_decompose(anim_bone_m1, anim_bone_vpos, anim_bone_q1, anim_bone_vscale);
			mat4_decompose(anim_bone_m, anim_bone_vpos2, anim_bone_q2, anim_bone_vscale2);

			// Lerp
			vec4_lerp(anim_bone_v1, anim_bone_vpos, anim_bone_vpos2, s);
			vec4_lerp(anim_bone_v2, anim_bone_vscale, anim_bone_vscale2, s);
			quat_lerp(anim_bone_q3, anim_bone_q1, anim_bone_q2, s);

			// Compose
			mat4_from_quat(anim_bone_m, anim_bone_q3);
			mat4_scale(anim_bone_m, anim_bone_v2);
			anim_bone_m._30 = anim_bone_v1.x;
			anim_bone_m._31 = anim_bone_v1.y;
			anim_bone_m._32 = anim_bone_v1.z;
		}

		if (raw.abs_mats != null && i < raw.abs_mats.length) mat4_set_from(raw.abs_mats[i], anim_bone_m);
		if (raw.bone_children != null) anim_bone_update_bone_children(raw, bones[i], anim_bone_m);

		mat4_mult_mats(anim_bone_m, anim_bone_m, raw.data._skeleton_transforms_inv[i]);
		anim_bone_update_skin_buffer(raw, anim_bone_m, i);
	}
}

function anim_bone_update_skin_buffer(raw: anim_bone_t, m: mat4_t, i: i32) {
	// Dual quat skinning
	mat4_decompose(m, anim_bone_vpos, anim_bone_q1, anim_bone_vscale);
	quat_normalize(anim_bone_q1);
	quat_set(anim_bone_q2, anim_bone_vpos.x, anim_bone_vpos.y, anim_bone_vpos.z, 0.0);
	quat_mult_quats(anim_bone_q2, anim_bone_q2, anim_bone_q1);
	raw.skin_buffer[i * 8] = anim_bone_q1.x; // Real
	raw.skin_buffer[i * 8 + 1] = anim_bone_q1.y;
	raw.skin_buffer[i * 8 + 2] = anim_bone_q1.z;
	raw.skin_buffer[i * 8 + 3] = anim_bone_q1.w;
	raw.skin_buffer[i * 8 + 4] = anim_bone_q2.x * 0.5; // Dual
	raw.skin_buffer[i * 8 + 5] = anim_bone_q2.y * 0.5;
	raw.skin_buffer[i * 8 + 6] = anim_bone_q2.z * 0.5;
	raw.skin_buffer[i * 8 + 7] = anim_bone_q2.w * 0.5;
}

function anim_bone_get_bone(raw: anim_bone_t, name: string): obj_t {
	if (raw.skeleton_bones == null) return null;
	for (let b of raw.skeleton_bones) if (b.name == name) return b;
	return null;
}

function anim_bone_get_bone_index(raw: anim_bone_t, bone: obj_t, bones: obj_t[] = null): i32 {
	if (bones == null) bones = raw.skeleton_bones;
	if (bones != null) for (let i = 0; i < bones.length; ++i) if (bones[i] == bone) return i;
	return -1;
}

function anim_bone_get_bone_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	return raw.skeleton_mats != null ? raw.skeleton_mats[anim_bone_get_bone_index(raw, bone)] : null;
}

function anim_bone_get_bone_mat_blend(raw: anim_bone_t, bone: obj_t): mat4_t {
	return raw.skeleton_mats_blend != null ? raw.skeleton_mats_blend[anim_bone_get_bone_index(raw, bone)] : null;
}

function anim_bone_get_abs_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	// With applied blending
	if (raw.skeleton_mats == null) return null;
	if (raw.abs_mats == null) {
		raw.abs_mats = [];
		while (raw.abs_mats.length < raw.skeleton_mats.length) raw.abs_mats.push(mat4_identity());
	}
	return raw.abs_mats[anim_bone_get_bone_index(raw, bone)];
}

function anim_bone_get_world_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	if (raw.skeleton_mats == null) return null;
	if (raw.apply_parent == null) {
		raw.apply_parent = [];
		for (let m of raw.skeleton_mats) raw.apply_parent.push(true);
	}
	let i = anim_bone_get_bone_index(raw, bone);
	mat4_set_from(anim_bone_wm, raw.skeleton_mats[i]);
	anim_bone_mult_parents(raw, anim_bone_wm, i, raw.skeleton_bones, raw.skeleton_mats);
	// wm.setFrom(matsFast[i]); // TODO
	return anim_bone_wm;
}

function anim_bone_get_bone_len(raw: anim_bone_t, bone: obj_t): f32 {
	let refs = raw.data.skin.bone_ref_array;
	let lens = raw.data.skin.bone_len_array;
	for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i];
	return 0.0;
}

// Returns bone length with scale applied
function anim_bone_get_bone_abs_len(raw: anim_bone_t, bone: obj_t): f32 {
	let refs = raw.data.skin.bone_ref_array;
	let lens = raw.data.skin.bone_len_array;
	let scale = mat4_get_scale(raw.object.base.parent.transform.world).z;
	for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i] * scale;
	return 0.0;
}

// Returns bone matrix in world space
function anim_bone_get_abs_world_mat(raw: anim_bone_t, bone: obj_t): mat4_t {
	let wm = anim_bone_get_world_mat(raw, bone);
	mat4_mult_mat(wm, raw.object.base.parent.transform.world);
	return wm;
}

function anim_bone_solve_ik(raw: anim_bone_t, effector: obj_t, goal: vec4_t, precision = 0.01, maxIterations = 100, chainLenght = 100, rollAngle = 0.0) {
	// Array of bones to solve IK for, effector at 0
	let bones: obj_t[] = [];

	// Array of bones lengths, effector length at 0
	let lengths: f32[] = [];

	// Array of bones matrices in world coordinates, effector at 0
	let boneWorldMats: mat4_t[];

	let tempLoc = vec4_create();
	let tempRot = quat_create();
	let tempRot2 = quat_create();
	let tempScl = vec4_create();
	let roll = quat_from_euler(quat_create(), 0, rollAngle, 0);

	// Store all bones and lengths in array
	let tip = effector;
	bones.push(tip);
	lengths.push(anim_bone_get_bone_abs_len(raw, tip));
	let root = tip;

	while (root.parent != null) {
		if (bones.length > chainLenght - 1) break;
		bones.push(root.parent);
		lengths.push(anim_bone_get_bone_abs_len(raw, root.parent));
		root = root.parent;
	}

	// Root bone
	root = bones[bones.length - 1];

	// World matrix of root bone
	let rootWorldMat = mat4_clone(anim_bone_get_world_mat(raw, root));
	// World matrix of armature
	let armatureMat = mat4_clone(raw.object.base.parent.transform.world);
	// Apply armature transform to world matrix
	mat4_mult_mat(rootWorldMat, armatureMat);
	// Distance from root to goal
	let dist = vec4_dist(goal, mat4_get_loc(rootWorldMat));

	// Total bones length
	let totalLength: f32 = 0.0;
	for (let l of lengths) totalLength += l;

	// Unreachable distance
	if (dist > totalLength) {
		// Calculate unit vector from root to goal
		let newLook = vec4_clone(goal);
		vec4_sub(newLook, mat4_get_loc(rootWorldMat));
		vec4_normalize(newLook);

		// Rotate root bone to point at goal
		mat4_decompose(rootWorldMat, tempLoc, tempRot, tempScl);
		quat_from_to(tempRot2, vec4_normalize(mat4_look(rootWorldMat)), newLook);
		quat_mult(tempRot2, tempRot);
		quat_mult(tempRot2, roll);
		mat4_compose(rootWorldMat, tempLoc, tempRot2, tempScl);

		// Set bone matrix in local space from world space
		anim_bone_set_bone_mat_from_world_mat(raw, rootWorldMat, root);

		// Set child bone rotations to zero
		for (let i = 0; i < bones.length - 1; ++i) {
			mat4_decompose(anim_bone_get_bone_mat(raw, bones[i]), tempLoc, tempRot, tempScl);
			mat4_compose(anim_bone_get_bone_mat(raw, bones[i]), tempLoc, roll, tempScl);
		}
		return;
	}

	// Get all bone mats in world space
	boneWorldMats = anim_bone_get_world_mats_fast(raw, effector, bones.length);

	// Array of bone locations in world space, root location at [0]
	let boneWorldLocs: vec4_t[] = [];
	for (let b of boneWorldMats) boneWorldLocs.push(mat4_get_loc(b));

	// Solve FABRIK
	let vec = vec4_create();
	let startLoc = vec4_clone(boneWorldLocs[0]);
	let l = boneWorldLocs.length;
	let testLength = 0;

	for (let iter = 0; iter < maxIterations; ++iter) {
		// Backward
		vec4_set_from(vec, goal);
		vec4_sub(vec, boneWorldLocs[l - 1]);
		vec4_normalize(vec);
		vec4_mult(vec, lengths[0]);
		vec4_set_from(boneWorldLocs[l - 1], goal);
		vec4_sub(boneWorldLocs[l - 1], vec);

		for (let j = 1; j < l; ++j) {
			vec4_set_from(vec, boneWorldLocs[l - 1 - j]);
			vec4_sub(vec, boneWorldLocs[l - j]);
			vec4_normalize(vec);
			vec4_mult(vec, lengths[j]);
			vec4_set_from(boneWorldLocs[l - 1 - j], boneWorldLocs[l - j]);
			vec4_add(boneWorldLocs[l - 1 - j], vec);
		}

		// Forward
		vec4_set_from(boneWorldLocs[0], startLoc);
		for (let j = 1; j < l; ++j) {
			vec4_set_from(vec, boneWorldLocs[j]);
			vec4_sub(vec, boneWorldLocs[j - 1]);
			vec4_normalize(vec, );
			vec4_mult(vec, lengths[l - j]);
			vec4_set_from(boneWorldLocs[j], boneWorldLocs[j - 1]);
			vec4_add(boneWorldLocs[j], vec);
		}

		if (vec4_dist(boneWorldLocs[l - 1], goal) - lengths[0] <= precision) break;
	}

	// Correct rotations
	// Applying locations and rotations
	let tempLook = vec4_create();
	let tempLoc2 = vec4_create();

	for (let i = 0; i < l - 1; ++i){
		// Decompose matrix
		mat4_decompose(boneWorldMats[i], tempLoc, tempRot, tempScl);

		// Rotate to point to parent bone
		vec4_set_from(tempLoc2, boneWorldLocs[i + 1]);
		vec4_sub(tempLoc2, boneWorldLocs[i]);
		vec4_normalize(tempLoc2, );
		vec4_set_from(tempLook, mat4_look(boneWorldMats[i]));
		vec4_normalize(tempLook);
		quat_from_to(tempRot2, tempLook, tempLoc2);
		quat_mult(tempRot2, tempRot);
		quat_mult(tempRot2, roll);

		// Compose matrix with new rotation and location
		mat4_compose(boneWorldMats[i], boneWorldLocs[i], tempRot2, tempScl);

		// Set bone matrix in local space from world space
		anim_bone_set_bone_mat_from_world_mat(raw, boneWorldMats[i], bones[bones.length - 1 - i]);
	}

	// Decompose matrix
	mat4_decompose(boneWorldMats[l - 1], tempLoc, tempRot, tempScl);

	// Rotate to point to goal
	vec4_set_from(tempLoc2, goal);
	vec4_sub(tempLoc2, tempLoc);
	vec4_normalize(tempLoc2, );
	vec4_set_from(tempLook, mat4_look(boneWorldMats[l - 1]));
	vec4_normalize(tempLook);
	quat_from_to(tempRot2, tempLook, tempLoc2);
	quat_mult(tempRot2, tempRot);
	quat_mult(tempRot2, roll);

	// Compose matrix with new rotation and location
	mat4_compose(boneWorldMats[l - 1], boneWorldLocs[l - 1], tempRot2, tempScl);

	// Set bone matrix in local space from world space
	anim_bone_set_bone_mat_from_world_mat(raw, boneWorldMats[l - 1], bones[0]);
}

// Returns an array of bone matrices in world space
function anim_bone_get_world_mats_fast(raw: anim_bone_t, tip: obj_t, chain_len: i32): mat4_t[] {
	let wmArray: mat4_t[] = [];
	let armatureMat = raw.object.base.parent.transform.world;
	let root = tip;
	let numP = chain_len;
	for (let i = 0; i < chain_len; ++i) {
		let wm = anim_bone_get_abs_world_mat(raw, root);
		wmArray[chain_len - 1 - i] = mat4_clone(wm);
		root = root.parent;
		numP--;
	}

	// Root bone at [0]
	return wmArray;
}

// Set bone transforms in world space
function anim_bone_set_bone_mat_from_world_mat(raw: anim_bone_t, wm: mat4_t, bone: obj_t) {
	let invMat = mat4_identity();
	let tempMat = mat4_clone(wm);
	mat4_get_inv(invMat, raw.object.base.parent.transform.world);
	mat4_mult_mat(tempMat, invMat);
	let bones: obj_t[] = [];
	let pBone = bone;
	while (pBone.parent != null) {
		bones.push(pBone.parent);
		pBone = pBone.parent;
	}

	for (let i = 0; i < bones.length; ++i) {
		let x = bones.length - 1;
		mat4_get_inv(invMat, anim_bone_get_bone_mat(raw, bones[x - i]));
		mat4_mult_mat(tempMat, invMat);
	}

	mat4_set_from(anim_bone_get_bone_mat(raw, bone), tempMat);
}

function anim_bone_play(raw: anim_bone_t, action = "", onComplete: ()=>void = null, blend_time = 0.2, speed = 1.0, loop = true) {
	if (action != "") {
		blend_time > 0 ? anim_bone_set_action_blend(raw, action) : anim_bone_set_action(raw, action);
	}
	raw.base.blend_factor = 0.0;
}

function anim_bone_blend(raw: anim_bone_t, action1: string, action2: string, factor: f32) {
	if (factor == 0.0) {
		anim_bone_set_action(raw, action1);
		return;
	}
	anim_bone_set_action(raw, action2);
	anim_bone_set_action_blend(raw, action1);

	anim_blend_super(raw.base, action1, action2, factor);
}

function anim_bone_update(raw: anim_bone_t, delta: f32) {
	if (!raw.base.is_skinned && raw.skeleton_bones == null) {
		anim_bone_set_action(raw, raw.base.armature.actions[0].name);
	}
	if (raw.object != null && (!raw.object.base.visible || raw.object.base.culled)) return;
	if (raw.skeleton_bones == null || raw.skeleton_bones.length == 0) return;

	anim_update_super(raw.base, delta);

	if (raw.base.paused || raw.base.speed == 0.0) return;

	let lastBones = raw.skeleton_bones;
	for (let b of raw.skeleton_bones) {
		if (b.anim != null) {
			anim_update_track(raw.base, b.anim);
			break;
		}
	}
	// Action has been changed by onComplete
	if (lastBones != raw.skeleton_bones) return;

	for (let i = 0; i < raw.skeleton_bones.length; ++i) {
		if (!raw.skeleton_bones[i].is_ik_fk_only) anim_update_anim_sampled(raw.base, raw.skeleton_bones[i].anim, raw.skeleton_mats[i]);
	}
	if (raw.base.blend_time > 0 && raw.skeleton_bones_blend != null) {
		for (let b of raw.skeleton_bones_blend) {
			if (b.anim != null) {
				anim_update_track(raw.base, b.anim);
				break;
			}
		}
		for (let i = 0; i < raw.skeleton_bones_blend.length; ++i) {
			anim_update_anim_sampled(raw.base, raw.skeleton_bones_blend[i].anim, raw.skeleton_mats_blend[i]);
		}
	}

	// Do forward kinematics and inverse kinematics here
	if (raw.on_updates != null) {
		let i = 0;
		let l = raw.on_updates.length;
		while (i < l) {
			raw.on_updates[i]();
			l <= raw.on_updates.length ? i++ : l = raw.on_updates.length;
		}
	}

	// Calc absolute bones
	for (let i = 0; i < raw.skeleton_bones.length; ++i) {
		// Take bones with 0 parents first
		anim_bone_mult_parent(raw, raw.mats_fast_sort[i], raw.mats_fast, raw.skeleton_bones, raw.skeleton_mats);
	}
	if (raw.skeleton_bones_blend != null) {
		for (let i = 0; i < raw.skeleton_bones_blend.length; ++i) {
			anim_bone_mult_parent(raw, raw.mats_fast_blend_sort[i], raw.mats_fast_blend, raw.skeleton_bones_blend, raw.skeleton_mats_blend);
		}
	}

	if (raw.base.is_skinned) anim_bone_update_skin_gpu(raw);
	else anim_bone_update_bones_only(raw);
}

function anim_bone_total_frames(raw: anim_bone_t): i32 {
	if (raw.skeleton_bones == null) return 0;
	let track = raw.skeleton_bones[0].anim.tracks[0];
	return Math.floor(track.frames[track.frames.length - 1] - track.frames[0]);
}

///end
