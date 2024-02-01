
class TransformRaw {
	world: TMat4;
	localOnly = false;
	local: TMat4;
	loc: TVec4;
	rot: TQuat;
	scale: TVec4;
	scaleWorld: f32 = 1.0;
	worldUnpack: TMat4;
	dirty: bool;
	object: TBaseObject;
	dim: TVec4;
	radius: f32;

	boneParent: TMat4 = null;
	lastWorld: TMat4 = null;

	// Wrong order returned from getEuler(), store last state for animation
	_eulerX: f32;
	_eulerY: f32;
	_eulerZ: f32;

	// Animated delta transform
	dloc: TVec4 = null;
	drot: TQuat = null;
	dscale: TVec4 = null;
	_deulerX: f32;
	_deulerY: f32;
	_deulerZ: f32;
}

class Transform {

	static temp = Mat4.identity();
	static q = Quat.create();

	static create(object: TBaseObject): TransformRaw {
		let raw = new TransformRaw();
		raw.object = object;
		Transform.reset(raw);
		return raw;
	}

	static reset = (raw: TransformRaw) => {
		raw.world = Mat4.identity();
		raw.worldUnpack = Mat4.identity();
		raw.local = Mat4.identity();
		raw.loc = Vec4.create();
		raw.rot = Quat.create();
		raw.scale = Vec4.create(1.0, 1.0, 1.0);
		raw.dim = Vec4.create(2.0, 2.0, 2.0);
		raw.radius = 1.0;
		raw.dirty = true;
	}

	static update = (raw: TransformRaw) => {
		if (raw.dirty) Transform.buildMatrix(raw);
	}

	static composeDelta = (raw: TransformRaw) => {
		// Delta transform
		Vec4.addvecs(raw.dloc, raw.loc, raw.dloc);
		Vec4.addvecs(raw.dscale, raw.dscale, raw.scale);
		Quat.fromEuler(raw.drot, raw._deulerX, raw._deulerY, raw._deulerZ);
		Quat.multquats(raw.drot, raw.rot, raw.drot);
		Mat4.compose(raw.local, raw.dloc, raw.drot, raw.dscale);
	}

	static buildMatrix = (raw: TransformRaw) => {
		raw.dloc == null ? Mat4.compose(raw.local, raw.loc, raw.rot, raw.scale) : Transform.composeDelta(raw);

		if (raw.boneParent != null) Mat4.multmats(raw.local, raw.boneParent, raw.local);

		if (raw.object.parent != null && !raw.localOnly) {
			Mat4.multmats3x4(raw.world, raw.local, raw.object.parent.transform.world);
		}
		else {
			Mat4.setFrom(raw.world, raw.local);
		}

		Mat4.setFrom(raw.worldUnpack, raw.world);
		if (raw.scaleWorld != 1.0) {
			raw.worldUnpack._00 *= raw.scaleWorld;
			raw.worldUnpack._01 *= raw.scaleWorld;
			raw.worldUnpack._02 *= raw.scaleWorld;
			raw.worldUnpack._03 *= raw.scaleWorld;
			raw.worldUnpack._10 *= raw.scaleWorld;
			raw.worldUnpack._11 *= raw.scaleWorld;
			raw.worldUnpack._12 *= raw.scaleWorld;
			raw.worldUnpack._13 *= raw.scaleWorld;
			raw.worldUnpack._20 *= raw.scaleWorld;
			raw.worldUnpack._21 *= raw.scaleWorld;
			raw.worldUnpack._22 *= raw.scaleWorld;
			raw.worldUnpack._23 *= raw.scaleWorld;
		}

		Transform.computeDim(raw);

		// Update children
		for (let n of raw.object.children) {
			Transform.buildMatrix(n.transform);
		}

		raw.dirty = false;
	}

	static translate = (raw: TransformRaw, x: f32, y: f32, z: f32) => {
		raw.loc.x += x;
		raw.loc.y += y;
		raw.loc.z += z;
		Transform.buildMatrix(raw);
	}

	static setMatrix = (raw: TransformRaw, mat: TMat4) => {
		Mat4.setFrom(raw.local, mat);
		Transform.decompose(raw);
		Transform.buildMatrix(raw);
	}

	static multMatrix = (raw: TransformRaw, mat: TMat4) => {
		Mat4.multmat(raw.local, mat);
		Transform.decompose(raw);
		Transform.buildMatrix(raw);
	}

	static decompose = (raw: TransformRaw) => {
		Mat4.decompose(raw.local, raw.loc, raw.rot, raw.scale);
	}

	static rotate = (raw: TransformRaw, axis: TVec4, f: f32) => {
		Quat.fromAxisAngle(Transform.q, axis, f);
		Quat.multquats(raw.rot, Transform.q, raw.rot);
		Transform.buildMatrix(raw);
	}

	static move = (raw: TransformRaw, axis: TVec4, f = 1.0) => {
		Vec4.addf(raw.loc, axis.x * f, axis.y * f, axis.z * f);
		Transform.buildMatrix(raw);
	}

	static setRotation = (raw: TransformRaw, x: f32, y: f32, z: f32) => {
		Quat.fromEuler(raw.rot, x, y, z);
		raw._eulerX = x;
		raw._eulerY = y;
		raw._eulerZ = z;
		raw.dirty = true;
	}

	static computeRadius = (raw: TransformRaw) => {
		raw.radius = Math.sqrt(raw.dim.x * raw.dim.x + raw.dim.y * raw.dim.y + raw.dim.z * raw.dim.z);
	}

	static computeDim = (raw: TransformRaw) => {
		if (raw.object.raw == null) {
			Transform.computeRadius(raw);
			return;
		}
		let d = raw.object.raw.dimensions;
		if (d == null) Vec4.set(raw.dim, 2 * raw.scale.x, 2 * raw.scale.y, 2 * raw.scale.z);
		else Vec4.set(raw.dim, d[0] * raw.scale.x, d[1] * raw.scale.y, d[2] * raw.scale.z);
		Transform.computeRadius(raw);
	}

	static applyParentInverse = (raw: TransformRaw) => {
		let pt = raw.object.parent.transform;
		Transform.buildMatrix(pt);
		Mat4.getInverse(Transform.temp, pt.world);
		Mat4.multmat(raw.local, Transform.temp);
		Transform.decompose(raw);
		Transform.buildMatrix(raw);
	}

	static applyParent = (raw: TransformRaw) => {
		let pt = raw.object.parent.transform;
		Transform.buildMatrix(pt);
		Mat4.multmat(raw.local, pt.world);
		Transform.decompose(raw);
		Transform.buildMatrix(raw);
	}

	static look = (raw: TransformRaw): TVec4 => {
		return Mat4.look(raw.world);
	}

	static right = (raw: TransformRaw): TVec4 => {
		return Mat4.right(raw.world);
	}

	static up = (raw: TransformRaw): TVec4 => {
		return Mat4.up(raw.world);
	}

	static worldx = (raw: TransformRaw): f32 => {
		return raw.world._30;
	}

	static worldy = (raw: TransformRaw): f32 => {
		return raw.world._31;
	}

	static worldz = (raw: TransformRaw): f32 => {
		return raw.world._32;
	}
}
