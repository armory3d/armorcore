/// <reference path='./Vec4.ts'/>
/// <reference path='./Mat4.ts'/>

class LightObject extends BaseObject {

	data: LightData;

	V: Mat4 = Mat4.identity();
	P: Mat4 = null;
	VP: Mat4 = Mat4.identity();

	frustumPlanes: FrustumPlane[] = null;
	static m = Mat4.identity();
	static eye = new Vec4();

	constructor(data: LightData) {
		super();

		this.data = data;

		let type = data.raw.type;
		let fov = data.raw.fov;

		if (type == "sun") {
			this.P = Mat4.ortho(-1, 1, -1, 1, data.raw.near_plane, data.raw.far_plane);
		}
		else if (type == "point" || type == "area") {
			this.P = Mat4.persp(fov, 1, data.raw.near_plane, data.raw.far_plane);
		}
		else if (type == "spot") {
			this.P = Mat4.persp(fov, 1, data.raw.near_plane, data.raw.far_plane);
		}

		Scene.active.lights.push(this);
	}

	override remove = () => {
		if (Scene.active != null) array_remove(Scene.active.lights, this);
		let rp = RenderPath.active;
		if (rp.light == this) { rp.light = null; }
		if (rp.point == this) { rp.point = null; }
		else if (rp.sun == this) { rp.sun = null; }
		this.removeSuper();
	}

	buildMatrix = (camera: CameraObject) => {
		this.transform.buildMatrix();
		if (this.data.raw.type == "sun") { // Cover camera frustum
			this.V.getInverse(this.transform.world);
			this.updateViewFrustum(camera);
		}
		else { // Point, spot, area
			this.V.getInverse(this.transform.world);
			this.updateViewFrustum(camera);
		}
	}

	updateViewFrustum = (camera: CameraObject) => {
		this.VP.multmats(this.P, this.V);

		// Frustum culling enabled
		if (camera.data.raw.frustum_culling) {
			if (this.frustumPlanes == null) {
				this.frustumPlanes = [];
				for (let i = 0; i < 6; ++i) this.frustumPlanes.push(new FrustumPlane());
			}
			CameraObject.buildViewFrustum(this.VP, this.frustumPlanes);
		}
	}

	right = (): Vec4 => {
		return new Vec4(this.V._00, this.V._10, this.V._20);
	}

	up = (): Vec4 => {
		return new Vec4(this.V._01, this.V._11, this.V._21);
	}

	look = (): Vec4 => {
		return new Vec4(this.V._02, this.V._12, this.V._22);
	}
}
