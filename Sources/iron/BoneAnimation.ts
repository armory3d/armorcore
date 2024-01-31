
///if arm_skin

class BoneAnimationRaw {
	base: AnimationRaw;
	object: MeshObject;
	data: TMeshData;
	skinBuffer: Float32Array;
	skeletonBones: TObj[] = null;
	skeletonMats: Mat4[] = null;
	skeletonBonesBlend: TObj[] = null;
	skeletonMatsBlend: Mat4[] = null;
	absMats: Mat4[] = null;
	applyParent: bool[] = null;
	matsFast: Mat4[] = [];
	matsFastSort: i32[] = [];
	matsFastBlend: Mat4[] = [];
	matsFastBlendSort: i32[] = [];
	boneChildren: Map<string, BaseObject[]> = null; // Parented to bone
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
	static vpos = new Vec4();
	static vscl = new Vec4();
	static q1 = new Quat();
	static q2 = new Quat();
	static q3 = new Quat();
	static vpos2 = new Vec4();
	static vscl2 = new Vec4();
	static v1 = new Vec4();
	static v2 = new Vec4();

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

	static setSkin = (raw: BoneAnimationRaw, mo: MeshObject) => {
		raw.object = mo;
		raw.data = mo != null ? mo.data : null;
		raw.base.isSkinned = raw.data != null ? raw.data.skin != null : false;
		if (raw.base.isSkinned) {
			let boneSize = 8; // Dual-quat skinning
			raw.skinBuffer = new Float32Array(BoneAnimation.skinMaxBones * boneSize);
			for (let i = 0; i < raw.skinBuffer.length; ++i) raw.skinBuffer[i] = 0;
			// Rotation is already applied to skin at export
			raw.object.base.transform.rot.set(0, 0, 0, 1);
			raw.object.base.transform.buildMatrix();

			let refs = mo.base.parent.raw.bone_actions;
			if (refs != null && refs.length > 0) {
				Data.getSceneRaw(refs[0], (action: TSceneFormat) => { BoneAnimation.play(raw, action.name); });
			}
		}
	}

	static addBoneChild = (raw: BoneAnimationRaw, bone: string, o: BaseObject) => {
		if (raw.boneChildren == null) raw.boneChildren = new Map();
		let ar = raw.boneChildren.get(bone);
		if (ar == null) {
			ar = [];
			raw.boneChildren.set(bone, ar);
		}
		ar.push(o);
	}

	static removeBoneChild = (raw: BoneAnimationRaw, bone: string, o: BaseObject) => {
		if (raw.boneChildren != null) {
			let ar = raw.boneChildren.get(bone);
			if (ar != null) array_remove(ar, o);
		}
	}

	static updateBoneChildren = (raw: BoneAnimationRaw, bone: TObj, bm: Mat4) => {
		let ar = raw.boneChildren.get(bone.name);
		if (ar == null) return;
		for (let o of ar) {
			let t = o.transform;
			if (t.boneParent == null) t.boneParent = Mat4.identity();
			if (o.raw.parent_bone_tail != null) {
				if (o.raw.parent_bone_connected || raw.base.isSkinned) {
					let v = o.raw.parent_bone_tail;
					t.boneParent.initTranslate(v[0], v[1], v[2]);
					t.boneParent.multmat(bm);
				}
				else {
					let v = o.raw.parent_bone_tail_pose;
					t.boneParent.setFrom(bm);
					t.boneParent.translate(v[0], v[1], v[2]);
				}
			}
			else t.boneParent.setFrom(bm);
			t.buildMatrix();
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

	static multParent = (raw: BoneAnimationRaw, i: i32, fasts: Mat4[], bones: TObj[], mats: Mat4[]) => {
		let f = fasts[i];
		if (raw.applyParent != null && !raw.applyParent[i]) {
			f.setFrom(mats[i]);
			return;
		}
		let p = bones[i].parent;
		let bi = BoneAnimation.getBoneIndex(raw, p, bones);
		(p == null || bi == -1) ? f.setFrom(mats[i]) : f.multmats(fasts[bi], mats[i]);
	}

	static multParents = (raw: BoneAnimationRaw, m: Mat4, i: i32, bones: TObj[], mats: Mat4[]) => {
		let bone = bones[i];
		let p = bone.parent;
		while (p != null) {
			let i = BoneAnimation.getBoneIndex(raw, p, bones);
			if (i == -1) continue;
			m.multmat(mats[i]);
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
				BoneAnimation.m.setFrom(raw.matsFast[i]);
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
			BoneAnimation.m.setFrom(raw.matsFast[i]);

			if (raw.base.blendTime > 0 && raw.skeletonBonesBlend != null) {
				// Decompose
				BoneAnimation.m1.setFrom(raw.matsFastBlend[i]);
				BoneAnimation.m1.decompose(BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
				BoneAnimation.m.decompose(BoneAnimation.vpos2, BoneAnimation.q2, BoneAnimation.vscl2);

				// Lerp
				BoneAnimation.v1.lerp(BoneAnimation.vpos, BoneAnimation.vpos2, s);
				BoneAnimation.v2.lerp(BoneAnimation.vscl, BoneAnimation.vscl2, s);
				BoneAnimation.q3.lerp(BoneAnimation.q1, BoneAnimation.q2, s);

				// Compose
				BoneAnimation.m.fromQuat(BoneAnimation.q3);
				BoneAnimation.m.scale(BoneAnimation.v2);
				BoneAnimation.m._30 = BoneAnimation.v1.x;
				BoneAnimation.m._31 = BoneAnimation.v1.y;
				BoneAnimation.m._32 = BoneAnimation.v1.z;
			}

			if (raw.absMats != null && i < raw.absMats.length) raw.absMats[i].setFrom(BoneAnimation.m);
			if (raw.boneChildren != null) BoneAnimation.updateBoneChildren(raw, bones[i], BoneAnimation.m);

			BoneAnimation.m.multmats(BoneAnimation.m, raw.data._skeletonTransformsI[i]);
			BoneAnimation.updateSkinBuffer(raw, BoneAnimation.m, i);
		}
	}

	static updateSkinBuffer = (raw: BoneAnimationRaw, m: Mat4, i: i32) => {
		// Dual quat skinning
		m.decompose(BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
		BoneAnimation.q1.normalize();
		BoneAnimation.q2.set(BoneAnimation.vpos.x, BoneAnimation.vpos.y, BoneAnimation.vpos.z, 0.0);
		BoneAnimation.q2.multquats(BoneAnimation.q2, BoneAnimation.q1);
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

	static getBoneMat = (raw: BoneAnimationRaw, bone: TObj): Mat4 => {
		return raw.skeletonMats != null ? raw.skeletonMats[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getBoneMatBlend = (raw: BoneAnimationRaw, bone: TObj): Mat4 => {
		return raw.skeletonMatsBlend != null ? raw.skeletonMatsBlend[BoneAnimation.getBoneIndex(raw, bone)] : null;
	}

	static getAbsMat = (raw: BoneAnimationRaw, bone: TObj): Mat4 => {
		// With applied blending
		if (raw.skeletonMats == null) return null;
		if (raw.absMats == null) {
			raw.absMats = [];
			while (raw.absMats.length < raw.skeletonMats.length) raw.absMats.push(Mat4.identity());
		}
		return raw.absMats[BoneAnimation.getBoneIndex(raw, bone)];
	}

	static getWorldMat = (raw: BoneAnimationRaw, bone: TObj): Mat4 => {
		if (raw.skeletonMats == null) return null;
		if (raw.applyParent == null) {
			raw.applyParent = [];
			for (let m of raw.skeletonMats) raw.applyParent.push(true);
		}
		let i = BoneAnimation.getBoneIndex(raw, bone);
		BoneAnimation.wm.setFrom(raw.skeletonMats[i]);
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
		let scale = raw.object.base.parent.transform.world.getScale().z;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i] * scale;
		return 0.0;
	}

	// Returns bone matrix in world space
	static getAbsWorldMat = (raw: BoneAnimationRaw, bone: TObj): Mat4 => {
		let wm = BoneAnimation.getWorldMat(raw, bone);
		wm.multmat(raw.object.base.parent.transform.world);
		return wm;
	}

	static solveIK = (raw: BoneAnimationRaw, effector: TObj, goal: Vec4, precision = 0.01, maxIterations = 100, chainLenght = 100, rollAngle = 0.0) => {
		// Array of bones to solve IK for, effector at 0
		let bones: TObj[] = [];

		// Array of bones lengths, effector length at 0
		let lengths: f32[] = [];

		// Array of bones matrices in world coordinates, effector at 0
		let boneWorldMats: Mat4[];

		let tempLoc = new Vec4();
		let tempRot = new Quat();
		let tempRot2 = new Quat();
		let tempScl = new Vec4();
		let roll = new Quat().fromEuler(0, rollAngle, 0);

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
		let rootWorldMat = BoneAnimation.getWorldMat(raw, root).clone();
		// World matrix of armature
		let armatureMat = raw.object.base.parent.transform.world.clone();
		// Apply armature transform to world matrix
		rootWorldMat.multmat(armatureMat);
		// Distance from root to goal
		let dist = Vec4.distance(goal, rootWorldMat.getLoc());

		// Total bones length
		let totalLength: f32 = 0.0;
		for (let l of lengths) totalLength += l;

		// Unreachable distance
		if (dist > totalLength) {
			// Calculate unit vector from root to goal
			let newLook = goal.clone();
			newLook.sub(rootWorldMat.getLoc());
			newLook.normalize();

			// Rotate root bone to point at goal
			rootWorldMat.decompose(tempLoc, tempRot, tempScl);
			tempRot2.fromTo(rootWorldMat.look().normalize(), newLook);
			tempRot2.mult(tempRot);
			tempRot2.mult(roll);
			rootWorldMat.compose(tempLoc, tempRot2, tempScl);

			// Set bone matrix in local space from world space
			BoneAnimation.setBoneMatFromWorldMat(raw, rootWorldMat, root);

			// Set child bone rotations to zero
			for (let i = 0; i < bones.length - 1; ++i) {
				BoneAnimation.getBoneMat(raw, bones[i]).decompose(tempLoc, tempRot, tempScl);
				BoneAnimation.getBoneMat(raw, bones[i]).compose(tempLoc, roll, tempScl);
			}
			return;
		}

		// Get all bone mats in world space
		boneWorldMats = BoneAnimation.getWorldMatsFast(raw, effector, bones.length);

		// Array of bone locations in world space, root location at [0]
		let boneWorldLocs: Vec4[] = [];
		for (let b of boneWorldMats) boneWorldLocs.push(b.getLoc());

		// Solve FABRIK
		let vec = new Vec4();
		let startLoc = boneWorldLocs[0].clone();
		let l = boneWorldLocs.length;
		let testLength = 0;

		for (let iter = 0; iter < maxIterations; ++iter) {
			// Backward
			vec.setFrom(goal);
			vec.sub(boneWorldLocs[l - 1]);
			vec.normalize();
			vec.mult(lengths[0]);
			boneWorldLocs[l - 1].setFrom(goal);
			boneWorldLocs[l - 1].sub(vec);

			for (let j = 1; j < l; ++j) {
				vec.setFrom(boneWorldLocs[l - 1 - j]);
				vec.sub(boneWorldLocs[l - j]);
				vec.normalize();
				vec.mult(lengths[j]);
				boneWorldLocs[l - 1 - j].setFrom(boneWorldLocs[l - j]);
				boneWorldLocs[l - 1 - j].add(vec);
			}

			// Forward
			boneWorldLocs[0].setFrom(startLoc);
			for (let j = 1; j < l; ++j) {
				vec.setFrom(boneWorldLocs[j]);
				vec.sub(boneWorldLocs[j - 1]);
				vec.normalize();
				vec.mult(lengths[l - j]);
				boneWorldLocs[j].setFrom(boneWorldLocs[j - 1]);
				boneWorldLocs[j].add(vec);
			}

			if (Vec4.distance(boneWorldLocs[l - 1], goal) - lengths[0] <= precision) break;
		}

		// Correct rotations
		// Applying locations and rotations
		let tempLook = new Vec4();
		let tempLoc2 = new Vec4();

		for (let i = 0; i < l - 1; ++i){
			// Decompose matrix
			boneWorldMats[i].decompose(tempLoc, tempRot, tempScl);

			// Rotate to point to parent bone
			tempLoc2.setFrom(boneWorldLocs[i + 1]);
			tempLoc2.sub(boneWorldLocs[i]);
			tempLoc2.normalize();
			tempLook.setFrom(boneWorldMats[i].look());
			tempLook.normalize();
			tempRot2.fromTo(tempLook, tempLoc2);
			tempRot2.mult(tempRot);
			tempRot2.mult(roll);

			// Compose matrix with new rotation and location
			boneWorldMats[i].compose(boneWorldLocs[i], tempRot2, tempScl);

			// Set bone matrix in local space from world space
			BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[i], bones[bones.length - 1 - i]);
		}

		// Decompose matrix
		boneWorldMats[l - 1].decompose(tempLoc, tempRot, tempScl);

		// Rotate to point to goal
		tempLoc2.setFrom(goal);
		tempLoc2.sub(tempLoc);
		tempLoc2.normalize();
		tempLook.setFrom(boneWorldMats[l - 1].look());
		tempLook.normalize();
		tempRot2.fromTo(tempLook, tempLoc2);
		tempRot2.mult(tempRot);
		tempRot2.mult(roll);

		// Compose matrix with new rotation and location
		boneWorldMats[l - 1].compose(boneWorldLocs[l - 1], tempRot2, tempScl);

		// Set bone matrix in local space from world space
		BoneAnimation.setBoneMatFromWorldMat(raw, boneWorldMats[l - 1], bones[0]);
	}

	// Returns an array of bone matrices in world space
	static getWorldMatsFast = (raw: BoneAnimationRaw, tip: TObj, chainLength: i32): Mat4[] => {
		let wmArray: Mat4[] = [];
		let armatureMat = raw.object.base.parent.transform.world;
		let root = tip;
		let numP = chainLength;
		for (let i = 0; i < chainLength; ++i) {
			let wm = BoneAnimation.getAbsWorldMat(raw, root);
			wmArray[chainLength - 1 - i] = wm.clone();
			root = root.parent;
			numP--;
		}

		// Root bone at [0]
		return wmArray;
	}

	// Set bone transforms in world space
	static setBoneMatFromWorldMat = (raw: BoneAnimationRaw, wm: Mat4, bone: TObj) => {
		let invMat = Mat4.identity();
		let tempMat = wm.clone();
		invMat.getInverse(raw.object.base.parent.transform.world);
		tempMat.multmat(invMat);
		let bones: TObj[] = [];
		let pBone = bone;
		while (pBone.parent != null) {
			bones.push(pBone.parent);
			pBone = pBone.parent;
		}

		for (let i = 0; i < bones.length; ++i) {
			let x = bones.length - 1;
			invMat.getInverse(BoneAnimation.getBoneMat(raw, bones[x - i]));
			tempMat.multmat(invMat);
		}

		BoneAnimation.getBoneMat(raw, bone).setFrom(tempMat);
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
