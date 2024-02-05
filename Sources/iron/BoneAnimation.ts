
///if arm_skin

class BoneAnimationRaw {
	base: AnimationRaw;
	object: TMeshObject;
	data: mesh_data_t;
	skinBuffer: Float32Array;
	skeletonBones: obj_t[] = null;
	skeletonMats: mat4_t[] = null;
	skeletonBonesBlend: obj_t[] = null;
	skeletonMatsBlend: mat4_t[] = null;
	absMats: mat4_t[] = null;
	applyParent: bool[] = null;
	matsFast: mat4_t[] = [];
	matsFastSort: i32[] = [];
	matsFastBlend: mat4_t[] = [];
	matsFastBlendSort: i32[] = [];
	boneChildren: Map<string, TBaseObject[]> = null; // Parented to bone
	// Do inverse kinematics here
	onUpdates: (()=>void)[] = null;
}

class BoneAnimation {
	static skinMaxBones = 128;
	static m = mat4_identity(); // Skinning matrix
	static m1 = mat4_identity();
	static m2 = mat4_identity();
	static bm = mat4_identity(); // Absolute bone matrix
	static wm = mat4_identity();
	static vpos = vec4_create();
	static vscl = vec4_create();
	static q1 = quat_create();
	static q2 = quat_create();
	static q3 = quat_create();
	static vpos2 = vec4_create();
	static vscl2 = vec4_create();
	static v1 = vec4_create();
	static v2 = vec4_create();

	static create(armatureName = ""): BoneAnimationRaw {
		let raw = new BoneAnimationRaw();
		raw.base = Animation.create();
		raw.base.ext = raw;

		raw.base.isSampled = false;
		for (let a of scene_armatures) {
			if (a.name == armatureName) {
				raw.base.armature = a;
				break;
			}
		}
		return raw;
	}

	static getNumBones = (raw: BoneAnimationRaw): i32 => {
		if (raw.skeletonBones == null) return 0;
		return raw.skeletonBones.length;
	}

	static setSkin = (raw: BoneAnimationRaw, mo: TMeshObject) => {
		raw.object = mo;
		raw.data = mo != null ? mo.data : null;
		raw.base.isSkinned = raw.data != null ? raw.data.skin != null : false;
		if (raw.base.isSkinned) {
			let boneSize = 8; // Dual-quat skinning
			raw.skinBuffer = new Float32Array(BoneAnimation.skinMaxBones * boneSize);
			for (let i = 0; i < raw.skinBuffer.length; ++i) raw.skinBuffer[i] = 0;
			// Rotation is already applied to skin at export
			quat_set(raw.object.base.transform.rot, 0, 0, 0, 1);
			transform_build_matrix(raw.object.base.transform);

			let refs = mo.base.parent.raw.bone_actions;
			if (refs != null && refs.length > 0) {
				Data.getSceneRaw(refs[0], (action: scene_t) => { BoneAnimation.play(raw, action.name); });
			}
		}
	}

	static addBoneChild = (raw: BoneAnimationRaw, bone: string, o: TBaseObject) => {
		if (raw.boneChildren == null) raw.boneChildren = new Map();
		let ar = raw.boneChildren.get(bone);
		if (ar == null) {
			ar = [];
			raw.boneChildren.set(bone, ar);
		}
		ar.push(o);
	}

	static removeBoneChild = (raw: BoneAnimationRaw, bone: string, o: TBaseObject) => {
		if (raw.boneChildren != null) {
			let ar = raw.boneChildren.get(bone);
			if (ar != null) array_remove(ar, o);
		}
	}

	static updateBoneChildren = (raw: BoneAnimationRaw, bone: obj_t, bm: mat4_t) => {
		let ar = raw.boneChildren.get(bone.name);
		if (ar == null) return;
		for (let o of ar) {
			let t = o.transform;
			if (t.bone_parent == null) t.bone_parent = mat4_identity();
			if (o.raw.parent_bone_tail != null) {
				if (o.raw.parent_bone_connected || raw.base.isSkinned) {
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

	static numParents = (b: obj_t): i32 => {
		let i = 0;
		let p = b.parent;
		while (p != null) {
			i++;
			p = p.parent;
		}
		return i;
	}

	static setMats = (raw: BoneAnimationRaw) => {
		while (raw.matsFast.length < raw.skeletonBones.length) {
			raw.matsFast.push(mat4_identity());
			raw.matsFastSort.push(raw.matsFastSort.length);
		}
		// Calc bones with 0 parents first
		raw.matsFastSort.sort((a, b) => {
			let i = BoneAnimation.numParents(raw.skeletonBones[a]);
			let j = BoneAnimation.numParents(raw.skeletonBones[b]);
			return i < j ? -1 : i > j ? 1 : 0;
		});

		if (raw.skeletonBonesBlend != null) {
			while (raw.matsFastBlend.length < raw.skeletonBonesBlend.length) {
				raw.matsFastBlend.push(mat4_identity());
				raw.matsFastBlendSort.push(raw.matsFastBlendSort.length);
			}
			raw.matsFastBlendSort.sort((a, b) => {
				let i = BoneAnimation.numParents(raw.skeletonBonesBlend[a]);
				let j = BoneAnimation.numParents(raw.skeletonBonesBlend[b]);
				return i < j ? -1 : i > j ? 1 : 0;
			});
		}
	}

	static setAction = (raw: BoneAnimationRaw, action: string) => {
		if (raw.base.isSkinned) {
			raw.skeletonBones = raw.data._actions.get(action);
			raw.skeletonMats = raw.data._mats.get(action);
			raw.skeletonBonesBlend = null;
			raw.skeletonMatsBlend = null;
		}
		else {
			armature_init_mats(raw.base.armature);
			let a = armature_get_action(raw.base.armature, action);
			raw.skeletonBones = a.bones;
			raw.skeletonMats = a.mats;
		}
		BoneAnimation.setMats(raw);
	}

	static setActionBlend = (raw: BoneAnimationRaw, action: string) => {
		if (raw.base.isSkinned) {
			raw.skeletonBonesBlend = raw.skeletonBones;
			raw.skeletonMatsBlend = raw.skeletonMats;
			raw.skeletonBones = raw.data._actions.get(action);
			raw.skeletonMats = raw.data._mats.get(action);
		}
		else {
			armature_init_mats(raw.base.armature);
			let a = armature_get_action(raw.base.armature, action);
			raw.skeletonBones = a.bones;
			raw.skeletonMats = a.mats;
		}
		BoneAnimation.setMats(raw);
	}

	static multParent = (raw: BoneAnimationRaw, i: i32, fasts: mat4_t[], bones: obj_t[], mats: mat4_t[]) => {
		let f = fasts[i];
		if (raw.applyParent != null && !raw.applyParent[i]) {
			mat4_set_from(f, mats[i]);
			return;
		}
		let p = bones[i].parent;
		let bi = BoneAnimation.getBoneIndex(raw, p, bones);
		(p == null || bi == -1) ? mat4_set_from(f, mats[i]) : mat4_mult_mats(f, fasts[bi], mats[i]);
	}

	static multParents = (raw: BoneAnimationRaw, m: mat4_t, i: i32, bones: obj_t[], mats: mat4_t[]) => {
		let bone = bones[i];
		let p = bone.parent;
		while (p != null) {
			let i = BoneAnimation.getBoneIndex(raw, p, bones);
			if (i == -1) continue;
			mat4_mult_mat(m, mats[i]);
			p = p.parent;
		}
	}

	static notifyOnUpdate = (raw: BoneAnimationRaw, f: ()=>void) => {
		if (raw.onUpdates == null) raw.onUpdates = [];
		raw.onUpdates.push(f);
	}

	static removeUpdate = (raw: BoneAnimationRaw, f: ()=>void) => {
		array_remove(raw.onUpdates, f);
	}

	static updateBonesOnly = (raw: BoneAnimationRaw) => {
		if (raw.boneChildren != null) {
			for (let i = 0; i < raw.skeletonBones.length; ++i) {
				let b = raw.skeletonBones[i]; // TODO: blendTime > 0
				mat4_set_from(BoneAnimation.m, raw.matsFast[i]);
				BoneAnimation.updateBoneChildren(raw, b, BoneAnimation.m);
			}
		}
	}

	static updateSkinGpu = (raw: BoneAnimationRaw) => {
		let bones = raw.skeletonBones;

		let s: f32 = raw.base.blendCurrent / raw.base.blendTime;
		s = s * s * (3.0 - 2.0 * s); // Smoothstep
		if (raw.base.blendFactor != 0.0) s = 1.0 - raw.base.blendFactor;

		// Update skin buffer
		for (let i = 0; i < bones.length; ++i) {
			mat4_set_from(BoneAnimation.m, raw.matsFast[i]);

			if (raw.base.blendTime > 0 && raw.skeletonBonesBlend != null) {
				// Decompose
				mat4_set_from(BoneAnimation.m1, raw.matsFastBlend[i]);
				mat4_decompose(BoneAnimation.m1, BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
				mat4_decompose(BoneAnimation.m, BoneAnimation.vpos2, BoneAnimation.q2, BoneAnimation.vscl2);

				// Lerp
				vec4_lerp(BoneAnimation.v1, BoneAnimation.vpos, BoneAnimation.vpos2, s);
				vec4_lerp(BoneAnimation.v2, BoneAnimation.vscl, BoneAnimation.vscl2, s);
				quat_lerp(BoneAnimation.q3, BoneAnimation.q1, BoneAnimation.q2, s);

				// Compose
				mat4_from_quat(BoneAnimation.m, BoneAnimation.q3);
				mat4_scale(BoneAnimation.m, BoneAnimation.v2);
				BoneAnimation.m._30 = BoneAnimation.v1.x;
				BoneAnimation.m._31 = BoneAnimation.v1.y;
				BoneAnimation.m._32 = BoneAnimation.v1.z;
			}

			if (raw.absMats != null && i < raw.absMats.length) mat4_set_from(raw.absMats[i], BoneAnimation.m);
			if (raw.boneChildren != null) BoneAnimation.updateBoneChildren(raw, bones[i], BoneAnimation.m);

			mat4_mult_mats(BoneAnimation.m, BoneAnimation.m, raw.data._skeleton_transforms_inv[i]);
			BoneAnimation.updateSkinBuffer(raw, BoneAnimation.m, i);
		}
	}

	static updateSkinBuffer = (raw: BoneAnimationRaw, m: mat4_t, i: i32) => {
		// Dual quat skinning
		mat4_decompose(m, BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
		quat_normalize(BoneAnimation.q1);
		quat_set(BoneAnimation.q2, BoneAnimation.vpos.x, BoneAnimation.vpos.y, BoneAnimation.vpos.z, 0.0);
		quat_mult_quats(BoneAnimation.q2, BoneAnimation.q2, BoneAnimation.q1);
		raw.skinBuffer[i * 8] = BoneAnimation.q1.x; // Real
		raw.skinBuffer[i * 8 + 1] = BoneAnimation.q1.y;
		raw.skinBuffer[i * 8 + 2] = BoneAnimation.q1.z;
		raw.skinBuffer[i * 8 + 3] = BoneAnimation.q1.w;
		raw.skinBuffer[i * 8 + 4] = BoneAnimation.q2.x * 0.5; // Dual
		raw.skinBuffer[i * 8 + 5] = BoneAnimation.q2.y * 0.5;
		raw.skinBuffer[i * 8 + 6] = BoneAnimation.q2.z * 0.5;
		raw.skinBuffer[i * 8 + 7] = BoneAnimation.q2.w * 0.5;
	}

	static getBone = (raw: BoneAnimationRaw, name: string): obj_t => {
		if (raw.skeletonBones == null) return null;
		for (let b of raw.skeletonBones) if (b.name == name) return b;
		return null;
	}

	static getBoneIndex = (raw: BoneAnimationRaw, bone: obj_t, bones: obj_t[] = null): i32 => {
		if (bones == null) bones = raw.skeletonBones;
		if (bones != null) for (let i = 0; i < bones.length; ++i) if (bones[i] == bone) return i;
		return -1;
	}

	static getBoneMat = (raw: BoneAnimationRaw, bone: obj_t): mat4_t => {
		return raw.skeletonMats != null ? raw.skeletonMats[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getBoneMatBlend = (raw: BoneAnimationRaw, bone: obj_t): mat4_t => {
		return raw.skeletonMatsBlend != null ? raw.skeletonMatsBlend[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getAbsMat = (raw: BoneAnimationRaw, bone: obj_t): mat4_t => {
		// With applied blending
		if (raw.skeletonMats == null) return null;
		if (raw.absMats == null) {
			raw.absMats = [];
			while (raw.absMats.length < raw.skeletonMats.length) raw.absMats.push(mat4_identity());
		}
		return raw.absMats[BoneAnimation.getBoneIndex(raw, bone)];
	}

	static getWorldMat = (raw: BoneAnimationRaw, bone: obj_t): mat4_t => {
		if (raw.skeletonMats == null) return null;
		if (raw.applyParent == null) {
			raw.applyParent = [];
			for (let m of raw.skeletonMats) raw.applyParent.push(true);
		}
		let i = BoneAnimation.getBoneIndex(raw, bone);
		mat4_set_from(BoneAnimation.wm, raw.skeletonMats[i]);
		BoneAnimation.multParents(raw, BoneAnimation.wm, i, raw.skeletonBones, raw.skeletonMats);
		// BoneAnimation.wm.setFrom(matsFast[i]); // TODO
		return BoneAnimation.wm;
	}

	static getBoneLen = (raw: BoneAnimationRaw, bone: obj_t): f32 => {
		let refs = raw.data.skin.bone_ref_array;
		let lens = raw.data.skin.bone_len_array;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i];
		return 0.0;
	}

	// Returns bone length with scale applied
	static getBoneAbsLen = (raw: BoneAnimationRaw, bone: obj_t): f32 => {
		let refs = raw.data.skin.bone_ref_array;
		let lens = raw.data.skin.bone_len_array;
		let scale = mat4_get_scale(raw.object.base.parent.transform.world).z;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i] * scale;
		return 0.0;
	}

	// Returns bone matrix in world space
	static getAbsWorldMat = (raw: BoneAnimationRaw, bone: obj_t): mat4_t => {
		let wm = BoneAnimation.getWorldMat(raw, bone);
		mat4_mult_mat(wm, raw.object.base.parent.transform.world);
		return wm;
	}

	static solveIK = (raw: BoneAnimationRaw, effector: obj_t, goal: vec4_t, precision = 0.01, maxIterations = 100, chainLenght = 100, rollAngle = 0.0) => {
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
		lengths.push(BoneAnimation.getBoneAbsLen(raw, tip));
		let root = tip;

		while (root.parent != null) {
			if (bones.length > chainLenght - 1) break;
			bones.push(root.parent);
			lengths.push(BoneAnimation.getBoneAbsLen(raw, root.parent));
			root = root.parent;
		}

		// Root bone
		root = bones[bones.length - 1];

		// World matrix of root bone
		let rootWorldMat = mat4_clone(BoneAnimation.getWorldMat(raw, root));
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
			BoneAnimation.setBoneMatFromWorldMat(raw, rootWorldMat, root);

			// Set child bone rotations to zero
			for (let i = 0; i < bones.length - 1; ++i) {
				mat4_decompose(BoneAnimation.getBoneMat(raw, bones[i]), tempLoc, tempRot, tempScl);
				mat4_compose(BoneAnimation.getBoneMat(raw, bones[i]), tempLoc, roll, tempScl);
			}
			return;
		}

		// Get all bone mats in world space
		boneWorldMats = BoneAnimation.getWorldMatsFast(raw, effector, bones.length);

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
			BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[i], bones[bones.length - 1 - i]);
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
		BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[l - 1], bones[0]);
	}

	// Returns an array of bone matrices in world space
	static getWorldMatsFast = (raw: BoneAnimationRaw, tip: obj_t, chainLength: i32): mat4_t[] => {
		let wmArray: mat4_t[] = [];
		let armatureMat = raw.object.base.parent.transform.world;
		let root = tip;
		let numP = chainLength;
		for (let i = 0; i < chainLength; ++i) {
			let wm = BoneAnimation.getAbsWorldMat(raw, root);
			wmArray[chainLength - 1 - i] = mat4_clone(wm);
			root = root.parent;
			numP--;
		}

		// Root bone at [0]
		return wmArray;
	}

	// Set bone transforms in world space
	static setBoneMatFromWorldMat = (raw: BoneAnimationRaw, wm: mat4_t, bone: obj_t) => {
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
			mat4_get_inv(invMat, BoneAnimation.getBoneMat(raw, bones[x - i]));
			mat4_mult_mat(tempMat, invMat);
		}

		mat4_set_from(BoneAnimation.getBoneMat(raw, bone), tempMat);
	}

	static play = (raw: BoneAnimationRaw, action = "", onComplete: ()=>void = null, blendTime = 0.2, speed = 1.0, loop = true) => {
		if (action != "") {
			blendTime > 0 ? BoneAnimation.setActionBlend(raw, action) : BoneAnimation.setAction(raw, action);
		}
		raw.base.blendFactor = 0.0;
	}

	static blend = (raw: BoneAnimationRaw, action1: string, action2: string, factor: f32) => {
		if (factor == 0.0) {
			BoneAnimation.setAction(raw, action1);
			return;
		}
		BoneAnimation.setAction(raw, action2);
		BoneAnimation.setActionBlend(raw, action1);

		Animation.blendSuper(raw.base, action1, action2, factor);
	}

	static update = (raw: BoneAnimationRaw, delta: f32) => {
		if (!raw.base.isSkinned && raw.skeletonBones == null) {
			BoneAnimation.setAction(raw, raw.base.armature.actions[0].name);
		}
		if (raw.object != null && (!raw.object.base.visible || raw.object.base.culled)) return;
		if (raw.skeletonBones == null || raw.skeletonBones.length == 0) return;

		Animation.updateSuper(raw.base, delta);

		if (raw.base.paused || raw.base.speed == 0.0) return;

		let lastBones = raw.skeletonBones;
		for (let b of raw.skeletonBones) {
			if (b.anim != null) {
				Animation.updateTrack(raw.base, b.anim);
				break;
			}
		}
		// Action has been changed by onComplete
		if (lastBones != raw.skeletonBones) return;

		for (let i = 0; i < raw.skeletonBones.length; ++i) {
			if (!raw.skeletonBones[i].is_ik_fk_only) Animation.updateAnimSampled(raw.base, raw.skeletonBones[i].anim, raw.skeletonMats[i]);
		}
		if (raw.base.blendTime > 0 && raw.skeletonBonesBlend != null) {
			for (let b of raw.skeletonBonesBlend) {
				if (b.anim != null) {
					Animation.updateTrack(raw.base, b.anim);
					break;
				}
			}
			for (let i = 0; i < raw.skeletonBonesBlend.length; ++i) {
				Animation.updateAnimSampled(raw.base, raw.skeletonBonesBlend[i].anim, raw.skeletonMatsBlend[i]);
			}
		}

		// Do forward kinematics and inverse kinematics here
		if (raw.onUpdates != null) {
			let i = 0;
			let l = raw.onUpdates.length;
			while (i < l) {
				raw.onUpdates[i]();
				l <= raw.onUpdates.length ? i++ : l = raw.onUpdates.length;
			}
		}

		// Calc absolute bones
		for (let i = 0; i < raw.skeletonBones.length; ++i) {
			// Take bones with 0 parents first
			BoneAnimation.multParent(raw, raw.matsFastSort[i], raw.matsFast, raw.skeletonBones, raw.skeletonMats);
		}
		if (raw.skeletonBonesBlend != null) {
			for (let i = 0; i < raw.skeletonBonesBlend.length; ++i) {
				BoneAnimation.multParent(raw, raw.matsFastBlendSort[i], raw.matsFastBlend, raw.skeletonBonesBlend, raw.skeletonMatsBlend);
			}
		}

		if (raw.base.isSkinned) BoneAnimation.updateSkinGpu(raw);
		else BoneAnimation.updateBonesOnly(raw);
	}

	static totalFrames = (raw: BoneAnimationRaw): i32 => {
		if (raw.skeletonBones == null) return 0;
		let track = raw.skeletonBones[0].anim.tracks[0];
		return Math.floor(track.frames[track.frames.length - 1] - track.frames[0]);
	}
}

///end
