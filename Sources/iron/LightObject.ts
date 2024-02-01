/// <reference path='./Vec4.ts'/>
/// <reference path='./Mat4.ts'/>

class TLightObject {
	base: TBaseObject;
	data: TLightData;
	V: TMat4 = Mat4.identity();
	P: TMat4 = null;
	VP: TMat4 = Mat4.identity();
	frustumPlanes: TFrustumPlane[] = null;
}

class LightObject {
	static m = Mat4.identity();
	static eye = Vec4.create();

	static create(data: TLightData): TLightObject {
		let raw = new TLightObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;
		raw.data = data;

		let type = data.type;
		let fov = data.fov;

		if (type == "sun") {
			raw.P = Mat4.ortho(-1, 1, -1, 1, data.near_plane, data.far_plane);
		}
		else if (type == "point" || type == "area") {
			raw.P = Mat4.persp(fov, 1, data.near_plane, data.far_plane);
		}
		else if (type == "spot") {
			raw.P = Mat4.persp(fov, 1, data.near_plane, data.far_plane);
		}

		Scene.lights.push(raw);
		return raw;
	}

	static remove = (raw: TLightObject) => {
		array_remove(Scene.lights, raw);
		if (RenderPath.light == raw) { RenderPath.light = null; }
		if (RenderPath.point == raw) { RenderPath.point = null; }
		else if (RenderPath.sun == raw) { RenderPath.sun = null; }

		BaseObject.removeSuper(raw.base);
	}

	static buildMatrix = (raw: TLightObject, camera: TCameraObject) => {
		Transform.buildMatrix(raw.base.transform);
		if (raw.data.type == "sun") { // Cover camera frustum
			Mat4.getInverse(raw.V, raw.base.transform.world);
			LightObject.updateViewFrustum(raw, camera);
		}
		else { // Point, spot, area
			Mat4.getInverse(raw.V, raw.base.transform.world);
			LightObject.updateViewFrustum(raw, camera);
		}
	}

	static updateViewFrustum = (raw: TLightObject, camera: TCameraObject) => {
		Mat4.multmats(raw.VP, raw.P, raw.V);

		// Frustum culling enabled
		if (camera.data.frustum_culling) {
			if (raw.frustumPlanes == null) {
				raw.frustumPlanes = [];
				for (let i = 0; i < 6; ++i) raw.frustumPlanes.push(new TFrustumPlane());
			}
			CameraObject.buildViewFrustum(raw.VP, raw.frustumPlanes);
		}
	}

	static right = (raw: TLightObject): TVec4 => {
		return Vec4.create(raw.V._00, raw.V._10, raw.V._20);
	}

	static up = (raw: TLightObject): TVec4 => {
		return Vec4.create(raw.V._01, raw.V._11, raw.V._21);
	}

	static look = (raw: TLightObject): TVec4 => {
		return Vec4.create(raw.V._02, raw.V._12, raw.V._22);
	}
}
