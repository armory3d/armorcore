
///if arm_skin

class BoneAnimationRaw {
	base: AnimationRaw;
	object: TMeshObject;
	data: TMeshData;
	skinBuffer: Float32Array;
	skeletonBones: TObj[] = null;
	skeletonMats: TMat4[] = null;
	skeletonBonesBlend: TObj[] = null;
	skeletonMatsBlend: TMat4[] = null;
	absMats: TMat4[] = null;
	applyParent: bool[] = null;
	matsFast: TMat4[] = [];
	matsFastSort: i32[] = [];
	matsFastBlend: TMat4[] = [];
	matsFastBlendSort: i32[] = [];
	boneChildren: Map<string, TBaseObject[]> = null; // Parented to bone
	// Do inverse kinematics here
	onUpdates: (()=>void)[] = null;
}

class BoneAnimation {
	static skinMaxBones = 128;
	static m = Mat4.identity(); // Skinning matrix
	static m1 = Mat4.identity();
	static m2 = Mat4.identity();
	static bm = Mat4.identity(); // Absolute bone matrix
	static wm = Mat4.identity();
	static vpos = Vec4.create();
	static vscl = Vec4.create();
	static q1 = Quat.create();
	static q2 = Quat.create();
	static q3 = Quat.create();
	static vpos2 = Vec4.create();
	static vscl2 = Vec4.create();
	static v1 = Vec4.create();
	static v2 = Vec4.create();

	static create(armatureName = ""): BoneAnimationRaw {
		let raw = new BoneAnimationRaw();
		raw.base = Animation.create();
		raw.base.ext = raw;

		raw.base.isSampled = false;
		for (let a of Scene.armatures) {
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
			Quat.set(raw.object.base.transform.rot, 0, 0, 0, 1);
			Transform.buildMatrix(raw.object.base.transform);

			let refs = mo.base.parent.raw.bone_actions;
			if (refs != null && refs.length > 0) {
				Data.getSceneRaw(refs[0], (action: TSceneFormat) => { BoneAnimation.play(raw, action.name); });
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

	static updateBoneChildren = (raw: BoneAnimationRaw, bone: TObj, bm: TMat4) => {
		let ar = raw.boneChildren.get(bone.name);
		if (ar == null) return;
		for (let o of ar) {
			let t = o.transform;
			if (t.boneParent == null) t.boneParent = Mat4.identity();
			if (o.raw.parent_bone_tail != null) {
				if (o.raw.parent_bone_connected || raw.base.isSkinned) {
					let v = o.raw.parent_bone_tail;
					Mat4.initTranslate(t.boneParent, v[0], v[1], v[2]);
					Mat4.multmat(t.boneParent, bm);
				}
				else {
					let v = o.raw.parent_bone_tail_pose;
					Mat4.setFrom(t.boneParent, bm);
					Mat4.translate(t.boneParent, v[0], v[1], v[2]);
				}
			}
			else Mat4.setFrom(t.boneParent, bm);
			Transform.buildMatrix(t);
		}
	}

	static numParents = (b: TObj): i32 => {
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
			raw.matsFast.push(Mat4.identity());
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
				raw.matsFastBlend.push(Mat4.identity());
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
			Armature.initMats(raw.base.armature);
			let a = Armature.getAction(raw.base.armature, action);
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
			Armature.initMats(raw.base.armature);
			let a = Armature.getAction(raw.base.armature, action);
			raw.skeletonBones = a.bones;
			raw.skeletonMats = a.mats;
		}
		BoneAnimation.setMats(raw);
	}

	static multParent = (raw: BoneAnimationRaw, i: i32, fasts: TMat4[], bones: TObj[], mats: TMat4[]) => {
		let f = fasts[i];
		if (raw.applyParent != null && !raw.applyParent[i]) {
			Mat4.setFrom(f, mats[i]);
			return;
		}
		let p = bones[i].parent;
		let bi = BoneAnimation.getBoneIndex(raw, p, bones);
		(p == null || bi == -1) ? Mat4.setFrom(f, mats[i]) : Mat4.multmats(f, fasts[bi], mats[i]);
	}

	static multParents = (raw: BoneAnimationRaw, m: TMat4, i: i32, bones: TObj[], mats: TMat4[]) => {
		let bone = bones[i];
		let p = bone.parent;
		while (p != null) {
			let i = BoneAnimation.getBoneIndex(raw, p, bones);
			if (i == -1) continue;
			Mat4.multmat(m, mats[i]);
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
				Mat4.setFrom(BoneAnimation.m, raw.matsFast[i]);
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
			Mat4.setFrom(BoneAnimation.m, raw.matsFast[i]);

			if (raw.base.blendTime > 0 && raw.skeletonBonesBlend != null) {
				// Decompose
				Mat4.setFrom(BoneAnimation.m1, raw.matsFastBlend[i]);
				Mat4.decompose(BoneAnimation.m1, BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
				Mat4.decompose(BoneAnimation.m, BoneAnimation.vpos2, BoneAnimation.q2, BoneAnimation.vscl2);

				// Lerp
				Vec4.lerp(BoneAnimation.v1, BoneAnimation.vpos, BoneAnimation.vpos2, s);
				Vec4.lerp(BoneAnimation.v2, BoneAnimation.vscl, BoneAnimation.vscl2, s);
				Quat.lerp(BoneAnimation.q3, BoneAnimation.q1, BoneAnimation.q2, s);

				// Compose
				Mat4.fromQuat(BoneAnimation.m, BoneAnimation.q3);
				Mat4.scale(BoneAnimation.m, BoneAnimation.v2);
				BoneAnimation.m._30 = BoneAnimation.v1.x;
				BoneAnimation.m._31 = BoneAnimation.v1.y;
				BoneAnimation.m._32 = BoneAnimation.v1.z;
			}

			if (raw.absMats != null && i < raw.absMats.length) Mat4.setFrom(raw.absMats[i], BoneAnimation.m);
			if (raw.boneChildren != null) BoneAnimation.updateBoneChildren(raw, bones[i], BoneAnimation.m);

			Mat4.multmats(BoneAnimation.m, BoneAnimation.m, raw.data._skeletonTransformsI[i]);
			BoneAnimation.updateSkinBuffer(raw, BoneAnimation.m, i);
		}
	}

	static updateSkinBuffer = (raw: BoneAnimationRaw, m: TMat4, i: i32) => {
		// Dual quat skinning
		Mat4.decompose(m, BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
		Quat.normalize(BoneAnimation.q1);
		Quat.set(BoneAnimation.q2, BoneAnimation.vpos.x, BoneAnimation.vpos.y, BoneAnimation.vpos.z, 0.0);
		Quat.multquats(BoneAnimation.q2, BoneAnimation.q2, BoneAnimation.q1);
		raw.skinBuffer[i * 8] = BoneAnimation.q1.x; // Real
		raw.skinBuffer[i * 8 + 1] = BoneAnimation.q1.y;
		raw.skinBuffer[i * 8 + 2] = BoneAnimation.q1.z;
		raw.skinBuffer[i * 8 + 3] = BoneAnimation.q1.w;
		raw.skinBuffer[i * 8 + 4] = BoneAnimation.q2.x * 0.5; // Dual
		raw.skinBuffer[i * 8 + 5] = BoneAnimation.q2.y * 0.5;
		raw.skinBuffer[i * 8 + 6] = BoneAnimation.q2.z * 0.5;
		raw.skinBuffer[i * 8 + 7] = BoneAnimation.q2.w * 0.5;
	}

	static getBone = (raw: BoneAnimationRaw, name: string): TObj => {
		if (raw.skeletonBones == null) return null;
		for (let b of raw.skeletonBones) if (b.name == name) return b;
		return null;
	}

	static getBoneIndex = (raw: BoneAnimationRaw, bone: TObj, bones: TObj[] = null): i32 => {
		if (bones == null) bones = raw.skeletonBones;
		if (bones != null) for (let i = 0; i < bones.length; ++i) if (bones[i] == bone) return i;
		return -1;
	}

	static getBoneMat = (raw: BoneAnimationRaw, bone: TObj): TMat4 => {
		return raw.skeletonMats != null ? raw.skeletonMats[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getBoneMatBlend = (raw: BoneAnimationRaw, bone: TObj): TMat4 => {
		return raw.skeletonMatsBlend != null ? raw.skeletonMatsBlend[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getAbsMat = (raw: BoneAnimationRaw, bone: TObj): TMat4 => {
		// With applied blending
		if (raw.skeletonMats == null) return null;
		if (raw.absMats == null) {
			raw.absMats = [];
			while (raw.absMats.length < raw.skeletonMats.length) raw.absMats.push(Mat4.identity());
		}
		return raw.absMats[BoneAnimation.getBoneIndex(raw, bone)];
	}

	static getWorldMat = (raw: BoneAnimationRaw, bone: TObj): TMat4 => {
		if (raw.skeletonMats == null) return null;
		if (raw.applyParent == null) {
			raw.applyParent = [];
			for (let m of raw.skeletonMats) raw.applyParent.push(true);
		}
		let i = BoneAnimation.getBoneIndex(raw, bone);
		Mat4.setFrom(BoneAnimation.wm, raw.skeletonMats[i]);
		BoneAnimation.multParents(raw, BoneAnimation.wm, i, raw.skeletonBones, raw.skeletonMats);
		// BoneAnimation.wm.setFrom(matsFast[i]); // TODO
		return BoneAnimation.wm;
	}

	static getBoneLen = (raw: BoneAnimationRaw, bone: TObj): f32 => {
		let refs = raw.data.skin.bone_ref_array;
		let lens = raw.data.skin.bone_len_array;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i];
		return 0.0;
	}

	// Returns bone length with scale applied
	static getBoneAbsLen = (raw: BoneAnimationRaw, bone: TObj): f32 => {
		let refs = raw.data.skin.bone_ref_array;
		let lens = raw.data.skin.bone_len_array;
		let scale = Mat4.getScale(raw.object.base.parent.transform.world).z;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i] * scale;
		return 0.0;
	}

	// Returns bone matrix in world space
	static getAbsWorldMat = (raw: BoneAnimationRaw, bone: TObj): TMat4 => {
		let wm = BoneAnimation.getWorldMat(raw, bone);
		Mat4.multmat(wm, raw.object.base.parent.transform.world);
		return wm;
	}

	static solveIK = (raw: BoneAnimationRaw, effector: TObj, goal: TVec4, precision = 0.01, maxIterations = 100, chainLenght = 100, rollAngle = 0.0) => {
		// Array of bones to solve IK for, effector at 0
		let bones: TObj[] = [];

		// Array of bones lengths, effector length at 0
		let lengths: f32[] = [];

		// Array of bones matrices in world coordinates, effector at 0
		let boneWorldMats: TMat4[];

		let tempLoc = Vec4.create();
		let tempRot = Quat.create();
		let tempRot2 = Quat.create();
		let tempScl = Vec4.create();
		let roll = Quat.fromEuler(Quat.create(), 0, rollAngle, 0);

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
		let rootWorldMat = Mat4.clone(BoneAnimation.getWorldMat(raw, root));
		// World matrix of armature
		let armatureMat = Mat4.clone(raw.object.base.parent.transform.world);
		// Apply armature transform to world matrix
		Mat4.multmat(rootWorldMat, armatureMat);
		// Distance from root to goal
		let dist = Vec4.distance(goal, Mat4.getLoc(rootWorldMat));

		// Total bones length
		let totalLength: f32 = 0.0;
		for (let l of lengths) totalLength += l;

		// Unreachable distance
		if (dist > totalLength) {
			// Calculate unit vector from root to goal
			let newLook = Vec4.clone(goal);
			Vec4.sub(newLook, Mat4.getLoc(rootWorldMat));
			Vec4.normalize(newLook);

			// Rotate root bone to point at goal
			Mat4.decompose(rootWorldMat, tempLoc, tempRot, tempScl);
			Quat.fromTo(tempRot2, Vec4.normalize(Mat4.look(rootWorldMat)), newLook);
			Quat.mult(tempRot2, tempRot);
			Quat.mult(tempRot2, roll);
			Mat4.compose(rootWorldMat, tempLoc, tempRot2, tempScl);

			// Set bone matrix in local space from world space
			BoneAnimation.setBoneMatFromWorldMat(raw, rootWorldMat, root);

			// Set child bone rotations to zero
			for (let i = 0; i < bones.length - 1; ++i) {
				Mat4.decompose(BoneAnimation.getBoneMat(raw, bones[i]), tempLoc, tempRot, tempScl);
				Mat4.compose(BoneAnimation.getBoneMat(raw, bones[i]), tempLoc, roll, tempScl);
			}
			return;
		}

		// Get all bone mats in world space
		boneWorldMats = BoneAnimation.getWorldMatsFast(raw, effector, bones.length);

		// Array of bone locations in world space, root location at [0]
		let boneWorldLocs: TVec4[] = [];
		for (let b of boneWorldMats) boneWorldLocs.push(Mat4.getLoc(b));

		// Solve FABRIK
		let vec = Vec4.create();
		let startLoc = Vec4.clone(boneWorldLocs[0]);
		let l = boneWorldLocs.length;
		let testLength = 0;

		for (let iter = 0; iter < maxIterations; ++iter) {
			// Backward
			Vec4.setFrom(vec, goal);
			Vec4.sub(vec, boneWorldLocs[l - 1]);
			Vec4.normalize(vec);
			Vec4.mult(vec, lengths[0]);
			Vec4.setFrom(boneWorldLocs[l - 1], goal);
			Vec4.sub(boneWorldLocs[l - 1], vec);

			for (let j = 1; j < l; ++j) {
				Vec4.setFrom(vec, boneWorldLocs[l - 1 - j]);
				Vec4.sub(vec, boneWorldLocs[l - j]);
				Vec4.normalize(vec);
				Vec4.mult(vec, lengths[j]);
				Vec4.setFrom(boneWorldLocs[l - 1 - j], boneWorldLocs[l - j]);
				Vec4.add(boneWorldLocs[l - 1 - j], vec);
			}

			// Forward
			Vec4.setFrom(boneWorldLocs[0], startLoc);
			for (let j = 1; j < l; ++j) {
				Vec4.setFrom(vec, boneWorldLocs[j]);
				Vec4.sub(vec, boneWorldLocs[j - 1]);
				Vec4.normalize(vec, );
				Vec4.mult(vec, lengths[l - j]);
				Vec4.setFrom(boneWorldLocs[j], boneWorldLocs[j - 1]);
				Vec4.add(boneWorldLocs[j], vec);
			}

			if (Vec4.distance(boneWorldLocs[l - 1], goal) - lengths[0] <= precision) break;
		}

		// Correct rotations
		// Applying locations and rotations
		let tempLook = Vec4.create();
		let tempLoc2 = Vec4.create();

		for (let i = 0; i < l - 1; ++i){
			// Decompose matrix
			Mat4.decompose(boneWorldMats[i], tempLoc, tempRot, tempScl);

			// Rotate to point to parent bone
			Vec4.setFrom(tempLoc2, boneWorldLocs[i + 1]);
			Vec4.sub(tempLoc2, boneWorldLocs[i]);
			Vec4.normalize(tempLoc2, );
			Vec4.setFrom(tempLook, Mat4.look(boneWorldMats[i]));
			Vec4.normalize(tempLook);
			Quat.fromTo(tempRot2, tempLook, tempLoc2);
			Quat.mult(tempRot2, tempRot);
			Quat.mult(tempRot2, roll);

			// Compose matrix with new rotation and location
			Mat4.compose(boneWorldMats[i], boneWorldLocs[i], tempRot2, tempScl);

			// Set bone matrix in local space from world space
			BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[i], bones[bones.length - 1 - i]);
		}

		// Decompose matrix
		Mat4.decompose(boneWorldMats[l - 1], tempLoc, tempRot, tempScl);

		// Rotate to point to goal
		Vec4.setFrom(tempLoc2, goal);
		Vec4.sub(tempLoc2, tempLoc);
		Vec4.normalize(tempLoc2, );
		Vec4.setFrom(tempLook, Mat4.look(boneWorldMats[l - 1]));
		Vec4.normalize(tempLook);
		Quat.fromTo(tempRot2, tempLook, tempLoc2);
		Quat.mult(tempRot2, tempRot);
		Quat.mult(tempRot2, roll);

		// Compose matrix with new rotation and location
		Mat4.compose(boneWorldMats[l - 1], boneWorldLocs[l - 1], tempRot2, tempScl);

		// Set bone matrix in local space from world space
		BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[l - 1], bones[0]);
	}

	// Returns an array of bone matrices in world space
	static getWorldMatsFast = (raw: BoneAnimationRaw, tip: TObj, chainLength: i32): TMat4[] => {
		let wmArray: TMat4[] = [];
		let armatureMat = raw.object.base.parent.transform.world;
		let root = tip;
		let numP = chainLength;
		for (let i = 0; i < chainLength; ++i) {
			let wm = BoneAnimation.getAbsWorldMat(raw, root);
			wmArray[chainLength - 1 - i] = Mat4.clone(wm);
			root = root.parent;
			numP--;
		}

		// Root bone at [0]
		return wmArray;
	}

	// Set bone transforms in world space
	static setBoneMatFromWorldMat = (raw: BoneAnimationRaw, wm: TMat4, bone: TObj) => {
		let invMat = Mat4.identity();
		let tempMat = Mat4.clone(wm);
		Mat4.getInverse(invMat, raw.object.base.parent.transform.world);
		Mat4.multmat(tempMat, invMat);
		let bones: TObj[] = [];
		let pBone = bone;
		while (pBone.parent != null) {
			bones.push(pBone.parent);
			pBone = pBone.parent;
		}

		for (let i = 0; i < bones.length; ++i) {
			let x = bones.length - 1;
			Mat4.getInverse(invMat, BoneAnimation.getBoneMat(raw, bones[x - i]));
			Mat4.multmat(tempMat, invMat);
		}

		Mat4.setFrom(BoneAnimation.getBoneMat(raw, bone), tempMat);
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
