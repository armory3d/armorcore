/// <reference path='./Vec4.ts'/>
/// <reference path='./Mat4.ts'/>

class LightObject {

	base: BaseObject;
	data: TLightData;
	V: Mat4 = Mat4.identity();
	P: Mat4 = null;
	VP: Mat4 = Mat4.identity();
	frustumPlanes: TFrustumPlane[] = null;

	static m = Mat4.identity();
	static eye = new Vec4();

	constructor(data: TLightData) {
		this.base = new BaseObject();
		this.base.ext = this;
		this.base.remove = this.remove;
		this.data = data;

		let type = data.type;
		let fov = data.fov;

		if (type == "sun") {
			this.P = Mat4.ortho(-1, 1, -1, 1, data.near_plane, data.far_plane);
		}
		else if (type == "point" || type == "area") {
			this.P = Mat4.persp(fov, 1, data.near_plane, data.far_plane);
		}
		else if (type == "spot") {
			this.P = Mat4.persp(fov, 1, data.near_plane, data.far_plane);
		}

		Scene.lights.push(this);
	}

	remove = () => {
		array_remove(Scene.lights, this);
		if (RenderPath.light == this) { RenderPath.light = null; }
		if (RenderPath.point == this) { RenderPath.point = null; }
		else if (RenderPath.sun == this) { RenderPath.sun = null; }
		this.base.removeSuper();
	}

	buildMatrix = (camera: CameraObject) => {
		this.base.transform.buildMatrix();
		if (this.data.type == "sun") { // Cover camera frustum
			this.V.getInverse(this.base.transform.world);
			this.updateViewFrustum(camera);
		}
		else { // Point, spot, area
			this.V.getInverse(this.base.transform.world);
			this.updateViewFrustum(camera);
		}
	}

	updateViewFrustum = (camera: CameraObject) => {
		this.VP.multmats(this.P, this.V);

		// Frustum culling enabled
		if (camera.data.frustum_culling) {
			if (this.frustumPlanes == null) {
				this.frustumPlanes = [];
				for (let i = 0; i < 6; ++i) this.frustumPlanes.push(new TFrustumPlane());
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
