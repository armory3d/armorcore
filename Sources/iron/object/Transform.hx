package iron.object;

import iron.math.Mat4;
import iron.math.Vec4;
import iron.math.Quat;

class Transform {

	public var world: Mat4;
	public var localOnly = false;
	public var local: Mat4;
	public var loc: Vec4;
	public var rot: Quat;
	public var scale: Vec4;
	public var scaleWorld: Float = 1.0;
	public var worldUnpack: Mat4;
	public var dirty: Bool;
	public var object: Object;
	public var dim: Vec4;
	public var radius: Float;

	static var temp = Mat4.identity();
	static var q = new Quat();

	var boneParent: Mat4 = null;
	var lastWorld: Mat4 = null;

	// Wrong order returned from getEuler(), store last state for animation
	var _eulerX: Float;
	var _eulerY: Float;
	var _eulerZ: Float;

	// Animated delta transform
	var dloc: Vec4 = null;
	var drot: Quat = null;
	var dscale: Vec4 = null;
	var _deulerX: Float;
	var _deulerY: Float;
	var _deulerZ: Float;

	public function new(object: Object) {
		this.object = object;
		reset();
	}

	public function reset() {
		world = Mat4.identity();
		worldUnpack = Mat4.identity();
		local = Mat4.identity();
		loc = new Vec4();
		rot = new Quat();
		scale = new Vec4(1.0, 1.0, 1.0);
		dim = new Vec4(2.0, 2.0, 2.0);
		radius = 1.0;
		dirty = true;
	}

	public function update() {
		if (dirty) buildMatrix();
	}

	function composeDelta() {
		// Delta transform
		dloc.addvecs(loc, dloc);
		dscale.addvecs(dscale, scale);
		drot.fromEuler(_deulerX, _deulerY, _deulerZ);
		drot.multquats(rot, drot);
		local.compose(dloc, drot, dscale);
	}

	public function buildMatrix() {
		dloc == null ? local.compose(loc, rot, scale) : composeDelta();

		if (boneParent != null) local.multmats(boneParent, local);

		if (object.parent != null && !localOnly) {
			world.multmats3x4(local, object.parent.transform.world);
		}
		else {
			world.setFrom(local);
		}

		worldUnpack.setFrom(world);
		if (scaleWorld != 1.0) {
			worldUnpack._00 *= scaleWorld;
			worldUnpack._01 *= scaleWorld;
			worldUnpack._02 *= scaleWorld;
			worldUnpack._03 *= scaleWorld;
			worldUnpack._10 *= scaleWorld;
			worldUnpack._11 *= scaleWorld;
			worldUnpack._12 *= scaleWorld;
			worldUnpack._13 *= scaleWorld;
			worldUnpack._20 *= scaleWorld;
			worldUnpack._21 *= scaleWorld;
			worldUnpack._22 *= scaleWorld;
			worldUnpack._23 *= scaleWorld;
		}

		computeDim();

		// Update children
		for (n in object.children) {
			n.transform.buildMatrix();
		}

		dirty = false;
	}

	public function translate(x: Float, y: Float, z: Float) {
		loc.x += x;
		loc.y += y;
		loc.z += z;
		buildMatrix();
	}

	public function setMatrix(mat: Mat4) {
		local.setFrom(mat);
		decompose();
		buildMatrix();
	}

	public function multMatrix(mat: Mat4) {
		local.multmat(mat);
		decompose();
		buildMatrix();
	}

	public function decompose() {
		local.decompose(loc, rot, scale);
	}

	public function rotate(axis: Vec4, f: Float) {
		q.fromAxisAngle(axis, f);
		rot.multquats(q, rot);
		buildMatrix();
	}

	public function move(axis: Vec4, f = 1.0) {
		loc.addf(axis.x * f, axis.y * f, axis.z * f);
		buildMatrix();
	}

	public function setRotation(x: Float, y: Float, z: Float) {
		rot.fromEuler(x, y, z);
		_eulerX = x;
		_eulerY = y;
		_eulerZ = z;
		dirty = true;
	}

	function computeRadius() {
		radius = Math.sqrt(dim.x * dim.x + dim.y * dim.y + dim.z * dim.z);
	}

	function computeDim() {
		if (object.raw == null) {
			computeRadius();
			return;
		}
		var d = object.raw.dimensions;
		if (d == null) dim.set(2 * scale.x, 2 * scale.y, 2 * scale.z);
		else dim.set(d[0] * scale.x, d[1] * scale.y, d[2] * scale.z);
		computeRadius();
	}

	public function applyParentInverse() {
		var pt = object.parent.transform;
		pt.buildMatrix();
		temp.getInverse(pt.world);
		this.local.multmat(temp);
		this.decompose();
		this.buildMatrix();
	}

	public function applyParent() {
		var pt = object.parent.transform;
		pt.buildMatrix();
		this.local.multmat(pt.world);
		this.decompose();
		this.buildMatrix();
	}

	public inline function look(): Vec4 {
		return world.look();
	}

	public inline function right(): Vec4 {
		return world.right();
	}

	public inline function up(): Vec4 {
		return world.up();
	}

	public inline function worldx(): Float {
		return world._30;
	}

	public inline function worldy(): Float {
		return world._31;
	}

	public inline function worldz(): Float {
		return world._32;
	}
}
