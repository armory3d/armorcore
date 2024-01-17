
class Transform {

	world: Mat4;
	localOnly = false;
	local: Mat4;
	loc: Vec4;
	rot: Quat;
	scale: Vec4;
	scaleWorld: f32 = 1.0;
	worldUnpack: Mat4;
	dirty: bool;
	object: BaseObject;
	dim: Vec4;
	radius: f32;

	static temp = Mat4.identity();
	static q = new Quat();

	boneParent: Mat4 = null;
	lastWorld: Mat4 = null;

	// Wrong order returned from getEuler(), store last state for animation
	_eulerX: f32;
	_eulerY: f32;
	_eulerZ: f32;

	// Animated delta transform
	dloc: Vec4 = null;
	drot: Quat = null;
	dscale: Vec4 = null;
	_deulerX: f32;
	_deulerY: f32;
	_deulerZ: f32;

	constructor(object: BaseObject) {
		this.object = object;
		this.reset();
	}

	reset = () => {
		this.world = Mat4.identity();
		this.worldUnpack = Mat4.identity();
		this.local = Mat4.identity();
		this.loc = new Vec4();
		this.rot = new Quat();
		this.scale = new Vec4(1.0, 1.0, 1.0);
		this.dim = new Vec4(2.0, 2.0, 2.0);
		this.radius = 1.0;
		this.dirty = true;
	}

	update = () => {
		if (this.dirty) this.buildMatrix();
	}

	composeDelta = () => {
		// Delta transform
		this.dloc.addvecs(this.loc, this.dloc);
		this.dscale.addvecs(this.dscale, this.scale);
		this.drot.fromEuler(this._deulerX, this._deulerY, this._deulerZ);
		this.drot.multquats(this.rot, this.drot);
		this.local.compose(this.dloc, this.drot, this.dscale);
	}

	buildMatrix = () => {
		this.dloc == null ? this.local.compose(this.loc, this.rot, this.scale) : this.composeDelta();

		if (this.boneParent != null) this.local.multmats(this.boneParent, this.local);

		if (this.object.parent != null && !this.localOnly) {
			this.world.multmats3x4(this.local, this.object.parent.transform.world);
		}
		else {
			this.world.setFrom(this.local);
		}

		this.worldUnpack.setFrom(this.world);
		if (this.scaleWorld != 1.0) {
			this.worldUnpack._00 *= this.scaleWorld;
			this.worldUnpack._01 *= this.scaleWorld;
			this.worldUnpack._02 *= this.scaleWorld;
			this.worldUnpack._03 *= this.scaleWorld;
			this.worldUnpack._10 *= this.scaleWorld;
			this.worldUnpack._11 *= this.scaleWorld;
			this.worldUnpack._12 *= this.scaleWorld;
			this.worldUnpack._13 *= this.scaleWorld;
			this.worldUnpack._20 *= this.scaleWorld;
			this.worldUnpack._21 *= this.scaleWorld;
			this.worldUnpack._22 *= this.scaleWorld;
			this.worldUnpack._23 *= this.scaleWorld;
		}

		this.computeDim();

		// Update children
		for (let n of this.object.children) {
			n.transform.buildMatrix();
		}

		this.dirty = false;
	}

	translate = (x: f32, y: f32, z: f32) => {
		this.loc.x += x;
		this.loc.y += y;
		this.loc.z += z;
		this.buildMatrix();
	}

	setMatrix = (mat: Mat4) => {
		this.local.setFrom(mat);
		this.decompose();
		this.buildMatrix();
	}

	multMatrix = (mat: Mat4) => {
		this.local.multmat(mat);
		this.decompose();
		this.buildMatrix();
	}

	decompose = () => {
		this.local.decompose(this.loc, this.rot, this.scale);
	}

	rotate = (axis: Vec4, f: f32) => {
		Transform.q.fromAxisAngle(axis, f);
		this.rot.multquats(Transform.q, this.rot);
		this.buildMatrix();
	}

	move = (axis: Vec4, f = 1.0) => {
		this.loc.addf(axis.x * f, axis.y * f, axis.z * f);
		this.buildMatrix();
	}

	setRotation = (x: f32, y: f32, z: f32) => {
		this.rot.fromEuler(x, y, z);
		this._eulerX = x;
		this._eulerY = y;
		this._eulerZ = z;
		this.dirty = true;
	}

	computeRadius = () => {
		this.radius = Math.sqrt(this.dim.x * this.dim.x + this.dim.y * this.dim.y + this.dim.z * this.dim.z);
	}

	computeDim = () => {
		if (this.object.raw == null) {
			this.computeRadius();
			return;
		}
		let d = this.object.raw.dimensions;
		if (d == null) this.dim.set(2 * this.scale.x, 2 * this.scale.y, 2 * this.scale.z);
		else this.dim.set(d[0] * this.scale.x, d[1] * this.scale.y, d[2] * this.scale.z);
		this.computeRadius();
	}

	applyParentInverse = () => {
		let pt = this.object.parent.transform;
		pt.buildMatrix();
		Transform.temp.getInverse(pt.world);
		this.local.multmat(Transform.temp);
		this.decompose();
		this.buildMatrix();
	}

	applyParent = () => {
		let pt = this.object.parent.transform;
		pt.buildMatrix();
		this.local.multmat(pt.world);
		this.decompose();
		this.buildMatrix();
	}

	look = (): Vec4 => {
		return this.world.look();
	}

	right = (): Vec4 => {
		return this.world.right();
	}

	up = (): Vec4 => {
		return this.world.up();
	}

	worldx = (): f32 => {
		return this.world._30;
	}

	worldy = (): f32 => {
		return this.world._31;
	}

	worldz = (): f32 => {
		return this.world._32;
	}
}
