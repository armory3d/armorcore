package iron;

import js.lib.Float32Array;
import iron.System;
import iron.CameraObject;

class LightObject extends Object {

	public var data: LightData;

	public var V: Mat4 = Mat4.identity();
	public var P: Mat4 = null;
	public var VP: Mat4 = Mat4.identity();

	public var frustumPlanes: Array<FrustumPlane> = null;
	static var m = Mat4.identity();
	static var eye = new Vec4();

	public function new(data: LightData) {
		super();

		this.data = data;

		var type = data.raw.type;
		var fov = data.raw.fov;

		if (type == "sun") {
			P = Mat4.ortho(-1, 1, -1, 1, data.raw.near_plane, data.raw.far_plane);
		}
		else if (type == "point" || type == "area") {
			P = Mat4.persp(fov, 1, data.raw.near_plane, data.raw.far_plane);
		}
		else if (type == "spot") {
			P = Mat4.persp(fov, 1, data.raw.near_plane, data.raw.far_plane);
		}

		Scene.active.lights.push(this);
	}

	override public function remove() {
		if (Scene.active != null) Scene.active.lights.remove(this);
		final rp = RenderPath.active;
		if (rp.light == this) { rp.light = null; }
		if (rp.point == this) { rp.point = null; }
		else if (rp.sun == this) { rp.sun = null; }
		super.remove();
	}

	public function buildMatrix(camera: CameraObject) {
		transform.buildMatrix();
		if (data.raw.type == "sun") { // Cover camera frustum
			V.getInverse(transform.world);
			updateViewFrustum(camera);
		}
		else { // Point, spot, area
			V.getInverse(transform.world);
			updateViewFrustum(camera);
		}
	}

	function updateViewFrustum(camera: CameraObject) {
		VP.multmats(P, V);

		// Frustum culling enabled
		if (camera.data.raw.frustum_culling) {
			if (frustumPlanes == null) {
				frustumPlanes = [];
				for (i in 0...6) frustumPlanes.push(new FrustumPlane());
			}
			CameraObject.buildViewFrustum(VP, frustumPlanes);
		}
	}

	public inline function right(): Vec4 {
		return new Vec4(V._00, V._10, V._20);
	}

	public inline function up(): Vec4 {
		return new Vec4(V._01, V._11, V._21);
	}

	public inline function look(): Vec4 {
		return new Vec4(V._02, V._12, V._22);
	}
}
