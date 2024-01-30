
///if arm_skin

class BoneAnimation extends Animation {

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

	constructor(armatureName = "") {
		super();
		this.isSampled = false;
		for (let a of Scene.armatures) {
			if (a.name == armatureName) {
				this.armature = a;
				break;
			}
		}
	}

	getNumBones = (): i32 => {
		if (this.skeletonBones == null) return 0;
		return this.skeletonBones.length;
	}

	setSkin = (mo: MeshObject) => {
		this.object = mo;
		this.data = mo != null ? mo.data : null;
		this.isSkinned = this.data != null ? this.data.skin != null : false;
		if (this.isSkinned) {
			let boneSize = 8; // Dual-quat skinning
			this.skinBuffer = new Float32Array(BoneAnimation.skinMaxBones * boneSize);
			for (let i = 0; i < this.skinBuffer.length; ++i) this.skinBuffer[i] = 0;
			// Rotation is already applied to skin at export
			this.object.transform.rot.set(0, 0, 0, 1);
			this.object.transform.buildMatrix();

			let refs = mo.parent.raw.bone_actions;
			if (refs != null && refs.length > 0) {
				Data.getSceneRaw(refs[0], (action: TSceneFormat) => { this.play(action.name); });
			}
		}
	}

	addBoneChild = (bone: string, o: BaseObject) => {
		if (this.boneChildren == null) this.boneChildren = new Map();
		let ar = this.boneChildren.get(bone);
		if (ar == null) {
			ar = [];
			this.boneChildren.set(bone, ar);
		}
		ar.push(o);
	}

	removeBoneChild = (bone: string, o: BaseObject) => {
		if (this.boneChildren != null) {
			let ar = this.boneChildren.get(bone);
			if (ar != null) array_remove(ar, o);
		}
	}

	updateBoneChildren = (bone: TObj, bm: Mat4) => {
		let ar = this.boneChildren.get(bone.name);
		if (ar == null) return;
		for (let o of ar) {
			let t = o.transform;
			if (t.boneParent == null) t.boneParent = Mat4.identity();
			if (o.raw.parent_bone_tail != null) {
				if (o.raw.parent_bone_connected || this.isSkinned) {
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

	numParents = (b: TObj): i32 => {
		let i = 0;
		let p = b.parent;
		while (p != null) {
			i++;
			p = p.parent;
		}
		return i;
	}

	setMats = () => {
		while (this.matsFast.length < this.skeletonBones.length) {
			this.matsFast.push(Mat4.identity());
			this.matsFastSort.push(this.matsFastSort.length);
		}
		// Calc bones with 0 parents first
		this.matsFastSort.sort((a, b) => {
			let i = this.numParents(this.skeletonBones[a]);
			let j = this.numParents(this.skeletonBones[b]);
			return i < j ? -1 : i > j ? 1 : 0;
		});

		if (this.skeletonBonesBlend != null) {
			while (this.matsFastBlend.length < this.skeletonBonesBlend.length) {
				this.matsFastBlend.push(Mat4.identity());
				this.matsFastBlendSort.push(this.matsFastBlendSort.length);
			}
			this.matsFastBlendSort.sort((a, b) => {
				let i = this.numParents(this.skeletonBonesBlend[a]);
				let j = this.numParents(this.skeletonBonesBlend[b]);
				return i < j ? -1 : i > j ? 1 : 0;
			});
		}
	}

	setAction = (action: string) => {
		if (this.isSkinned) {
			this.skeletonBones = this.data._actions.get(action);
			this.skeletonMats = this.data._mats.get(action);
			this.skeletonBonesBlend = null;
			this.skeletonMatsBlend = null;
		}
		else {
			this.armature.initMats();
			let a = this.armature.getAction(action);
			this.skeletonBones = a.bones;
			this.skeletonMats = a.mats;
		}
		this.setMats();
	}

	setActionBlend = (action: string) => {
		if (this.isSkinned) {
			this.skeletonBonesBlend = this.skeletonBones;
			this.skeletonMatsBlend = this.skeletonMats;
			this.skeletonBones = this.data._actions.get(action);
			this.skeletonMats = this.data._mats.get(action);
		}
		else {
			this.armature.initMats();
			let a = this.armature.getAction(action);
			this.skeletonBones = a.bones;
			this.skeletonMats = a.mats;
		}
		this.setMats();
	}

	multParent = (i: i32, fasts: Mat4[], bones: TObj[], mats: Mat4[]) => {
		let f = fasts[i];
		if (this.applyParent != null && !this.applyParent[i]) {
			f.setFrom(mats[i]);
			return;
		}
		let p = bones[i].parent;
		let bi = this.getBoneIndex(p, bones);
		(p == null || bi == -1) ? f.setFrom(mats[i]) : f.multmats(fasts[bi], mats[i]);
	}

	multParents = (m: Mat4, i: i32, bones: TObj[], mats: Mat4[]) => {
		let bone = bones[i];
		let p = bone.parent;
		while (p != null) {
			let i = this.getBoneIndex(p, bones);
			if (i == -1) continue;
			m.multmat(mats[i]);
			p = p.parent;
		}
	}

	notifyOnUpdate = (f: ()=>void) => {
		if (this.onUpdates == null) this.onUpdates = [];
		this.onUpdates.push(f);
	}

	removeUpdate = (f: ()=>void) => {
		array_remove(this.onUpdates, f);
	}

	updateBonesOnly = () => {
		if (this.boneChildren != null) {
			for (let i = 0; i < this.skeletonBones.length; ++i) {
				let b = this.skeletonBones[i]; // TODO: blendTime > 0
				BoneAnimation.m.setFrom(this.matsFast[i]);
				this.updateBoneChildren(b, BoneAnimation.m);
			}
		}
	}

	updateSkinGpu = () => {
		let bones = this.skeletonBones;

		let s: f32 = this.blendCurrent / this.blendTime;
		s = s * s * (3.0 - 2.0 * s); // Smoothstep
		if (this.blendFactor != 0.0) s = 1.0 - this.blendFactor;

		// Update skin buffer
		for (let i = 0; i < bones.length; ++i) {
			BoneAnimation.m.setFrom(this.matsFast[i]);

			if (this.blendTime > 0 && this.skeletonBonesBlend != null) {
				// Decompose
				BoneAnimation.m1.setFrom(this.matsFastBlend[i]);
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

			if (this.absMats != null && i < this.absMats.length) this.absMats[i].setFrom(BoneAnimation.m);
			if (this.boneChildren != null) this.updateBoneChildren(bones[i], BoneAnimation.m);

			BoneAnimation.m.multmats(BoneAnimation.m, this.data._skeletonTransformsI[i]);
			this.updateSkinBuffer(BoneAnimation.m, i);
		}
	}

	updateSkinBuffer = (m: Mat4, i: i32) => {
		// Dual quat skinning
		m.decompose(BoneAnimation.vpos, BoneAnimation.q1, BoneAnimation.vscl);
		BoneAnimation.q1.normalize();
		BoneAnimation.q2.set(BoneAnimation.vpos.x, BoneAnimation.vpos.y, BoneAnimation.vpos.z, 0.0);
		BoneAnimation.q2.multquats(BoneAnimation.q2, BoneAnimation.q1);
		this.skinBuffer[i * 8] = BoneAnimation.q1.x; // Real
		this.skinBuffer[i * 8 + 1] = BoneAnimation.q1.y;
		this.skinBuffer[i * 8 + 2] = BoneAnimation.q1.z;
		this.skinBuffer[i * 8 + 3] = BoneAnimation.q1.w;
		this.skinBuffer[i * 8 + 4] = BoneAnimation.q2.x * 0.5; // Dual
		this.skinBuffer[i * 8 + 5] = BoneAnimation.q2.y * 0.5;
		this.skinBuffer[i * 8 + 6] = BoneAnimation.q2.z * 0.5;
		this.skinBuffer[i * 8 + 7] = BoneAnimation.q2.w * 0.5;
	}

	getBone = (name: string): TObj => {
		if (this.skeletonBones == null) return null;
		for (let b of this.skeletonBones) if (b.name == name) return b;
		return null;
	}

	getBoneIndex = (bone: TObj, bones: TObj[] = null): i32 => {
		if (bones == null) bones = this.skeletonBones;
		if (bones != null) for (let i = 0; i < bones.length; ++i) if (bones[i] == bone) return i;
		return -1;
	}

	getBoneMat = (bone: TObj): Mat4 => {
		return this.skeletonMats != null ? this.skeletonMats[this.getBoneIndex(bone)] : null;
	}

	getBoneMatBlend = (bone: TObj): Mat4 => {
		return this.skeletonMatsBlend != null ? this.skeletonMatsBlend[this.getBoneIndex(bone)] : null;
	}

	getAbsMat = (bone: TObj): Mat4 => {
		// With applied blending
		if (this.skeletonMats == null) return null;
		if (this.absMats == null) {
			this.absMats = [];
			while (this.absMats.length < this.skeletonMats.length) this.absMats.push(Mat4.identity());
		}
		return this.absMats[this.getBoneIndex(bone)];
	}

	getWorldMat = (bone: TObj): Mat4 => {
		if (this.skeletonMats == null) return null;
		if (this.applyParent == null) {
			this.applyParent = [];
			for (let m of this.skeletonMats) this.applyParent.push(true);
		}
		let i = this.getBoneIndex(bone);
		BoneAnimation.wm.setFrom(this.skeletonMats[i]);
		this.multParents(BoneAnimation.wm, i, this.skeletonBones, this.skeletonMats);
		// BoneAnimation.wm.setFrom(matsFast[i]); // TODO
		return BoneAnimation.wm;
	}

	getBoneLen = (bone: TObj): f32 => {
		let refs = this.data.skin.bone_ref_array;
		let lens = this.data.skin.bone_len_array;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i];
		return 0.0;
	}

	// Returns bone length with scale applied
	getBoneAbsLen = (bone: TObj): f32 => {
		let refs = this.data.skin.bone_ref_array;
		let lens = this.data.skin.bone_len_array;
		let scale = this.object.parent.transform.world.getScale().z;
		for (let i = 0; i < refs.length; ++i) if (refs[i] == bone.name) return lens[i] * scale;
		return 0.0;
	}

	// Returns bone matrix in world space
	getAbsWorldMat = (bone: TObj): Mat4 => {
		let wm = this.getWorldMat(bone);
		wm.multmat(this.object.parent.transform.world);
		return wm;
	}

	solveIK = (effector: TObj, goal: Vec4, precision = 0.01, maxIterations = 100, chainLenght = 100, rollAngle = 0.0) => {
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
		lengths.push(this.getBoneAbsLen(tip));
		let root = tip;

		while (root.parent != null) {
			if (bones.length > chainLenght - 1) break;
			bones.push(root.parent);
			lengths.push(this.getBoneAbsLen(root.parent));
			root = root.parent;
		}

		// Root bone
		root = bones[bones.length - 1];

		// World matrix of root bone
		let rootWorldMat = this.getWorldMat(root).clone();
		// World matrix of armature
		let armatureMat = this.object.parent.transform.world.clone();
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
			this.setBoneMatFromWorldMat(rootWorldMat, root);

			// Set child bone rotations to zero
			for (let i = 0; i < bones.length - 1; ++i) {
				this.getBoneMat(bones[i]).decompose(tempLoc, tempRot, tempScl);
				this.getBoneMat(bones[i]).compose(tempLoc, roll, tempScl);
			}
			return;
		}

		// Get all bone mats in world space
		boneWorldMats = this.getWorldMatsFast(effector, bones.length);

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
			this.setBoneMatFromWorldMat(boneWorldMats[i], bones[bones.length - 1 - i]);
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
		this.setBoneMatFromWorldMat(boneWorldMats[l - 1], bones[0]);
	}

	// Returns an array of bone matrices in world space
	getWorldMatsFast = (tip: TObj, chainLength: i32): Mat4[] => {
		let wmArray: Mat4[] = [];
		let armatureMat = this.object.parent.transform.world;
		let root = tip;
		let numP = chainLength;
		for (let i = 0; i < chainLength; ++i) {
			let wm = this.getAbsWorldMat(root);
			wmArray[chainLength - 1 - i] = wm.clone();
			root = root.parent;
			numP--;
		}

		// Root bone at [0]
		return wmArray;
	}

	// Set bone transforms in world space
	setBoneMatFromWorldMat = (wm: Mat4, bone: TObj) => {
		let invMat = Mat4.identity();
		let tempMat = wm.clone();
		invMat.getInverse(this.object.parent.transform.world);
		tempMat.multmat(invMat);
		let bones: TObj[] = [];
		let pBone = bone;
		while (pBone.parent != null) {
			bones.push(pBone.parent);
			pBone = pBone.parent;
		}

		for (let i = 0; i < bones.length; ++i) {
			let x = bones.length - 1;
			invMat.getInverse(this.getBoneMat(bones[x - i]));
			tempMat.multmat(invMat);
		}

		this.getBoneMat(bone).setFrom(tempMat);
	}

	override play = (action = "", onComplete: ()=>void = null, blendTime = 0.2, speed = 1.0, loop = true) => {
		this.playSuper(action, onComplete, blendTime, speed, loop);
		if (action != "") {
			blendTime > 0 ? this.setActionBlend(action) : this.setAction(action);
		}
		this.blendFactor = 0.0;
	}

	override blend = (action1: string, action2: string, factor: f32) => {
		if (factor == 0.0) {
			this.setAction(action1);
			return;
		}
		this.setAction(action2);
		this.setActionBlend(action1);
		this.blendSuper(action1, action2, factor);
	}

	override update = (delta: f32) => {
		if (!this.isSkinned && this.skeletonBones == null) this.setAction(this.armature.actions[0].name);
		if (this.object != null && (!this.object.visible || this.object.culled)) return;
		if (this.skeletonBones == null || this.skeletonBones.length == 0) return;

		this.updateSuper(delta);
		if (this.paused || this.speed == 0.0) return;

		let lastBones = this.skeletonBones;
		for (let b of this.skeletonBones) {
			if (b.anim != null) {
				this.updateTrack(b.anim);
				break;
			}
		}
		// Action has been changed by onComplete
		if (lastBones != this.skeletonBones) return;

		for (let i = 0; i < this.skeletonBones.length; ++i) {
			if (!this.skeletonBones[i].is_ik_fk_only) this.updateAnimSampled(this.skeletonBones[i].anim, this.skeletonMats[i]);
		}
		if (this.blendTime > 0 && this.skeletonBonesBlend != null) {
			for (let b of this.skeletonBonesBlend) {
				if (b.anim != null) {
					this.updateTrack(b.anim);
					break;
				}
			}
			for (let i = 0; i < this.skeletonBonesBlend.length; ++i) {
				this.updateAnimSampled(this.skeletonBonesBlend[i].anim, this.skeletonMatsBlend[i]);
			}
		}

		// Do forward kinematics and inverse kinematics here
		if (this.onUpdates != null) {
			let i = 0;
			let l = this.onUpdates.length;
			while (i < l) {
				this.onUpdates[i]();
				l <= this.onUpdates.length ? i++ : l = this.onUpdates.length;
			}
		}

		// Calc absolute bones
		for (let i = 0; i < this.skeletonBones.length; ++i) {
			// Take bones with 0 parents first
			this.multParent(this.matsFastSort[i], this.matsFast, this.skeletonBones, this.skeletonMats);
		}
		if (this.skeletonBonesBlend != null) {
			for (let i = 0; i < this.skeletonBonesBlend.length; ++i) {
				this.multParent(this.matsFastBlendSort[i], this.matsFastBlend, this.skeletonBonesBlend, this.skeletonMatsBlend);
			}
		}

		if (this.isSkinned) this.updateSkinGpu();
		else this.updateBonesOnly();
	}

	override totalFrames = (): i32 => {
		if (this.skeletonBones == null) return 0;
		let track = this.skeletonBones[0].anim.tracks[0];
		return Math.floor(track.frames[track.frames.length - 1] - track.frames[0]);
	}
}

///end
