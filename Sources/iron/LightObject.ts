/// <reference path='./vec4.ts'/>
/// <reference path='./mat4.ts'/>

class TLightObject {
	base: TBaseObject;
	data: light_data_t;
	V: mat4_t = mat4_identity();
	P: mat4_t = null;
	VP: mat4_t = mat4_identity();
	frustumPlanes: TFrustumPlane[] = null;
}

class LightObject {
	static m = mat4_identity();
	static eye = vec4_create();

	static create(data: light_data_t): TLightObject {
		let raw = new TLightObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;
		raw.data = data;

		let type = data.type;
		let fov = data.fov;

		if (type == "sun") {
			raw.P = mat4_ortho(-1, 1, -1, 1, data.near_plane, data.far_plane);
		}
		else if (type == "point" || type == "area") {
			raw.P = mat4_persp(fov, 1, data.near_plane, data.far_plane);
		}
		else if (type == "spot") {
			raw.P = mat4_persp(fov, 1, data.near_plane, data.far_plane);
		}

		scene_lights.push(raw);
		return raw;
	}

	static remove = (raw: TLightObject) => {
		array_remove(scene_lights, raw);
		if (_render_path_light == raw) {_render_path_light = null; }
		if (_render_path_point == raw) {_render_path_point = null; }
		else if (_render_path_sun == raw) {_render_path_sun = null; }

		BaseObject.removeSuper(raw.base);
	}

	static buildMatrix = (raw: TLightObject, camera: TCameraObject) => {
		transform_build_matrix(raw.base.transform);
		if (raw.data.type == "sun") { // Cover camera frustum
			mat4_get_inv(raw.V, raw.base.transform.world);
			LightObject.updateViewFrustum(raw, camera);
		}
		else { // Point, spot, area
			mat4_get_inv(raw.V, raw.base.transform.world);
			LightObject.updateViewFrustum(raw, camera);
		}
	}

	static updateViewFrustum = (raw: TLightObject, camera: TCameraObject) => {
		mat4_mult_mats(raw.VP, raw.P, raw.V);

		// Frustum culling enabled
		if (camera.data.frustum_culling) {
			if (raw.frustumPlanes == null) {
				raw.frustumPlanes = [];
				for (let i = 0; i < 6; ++i) raw.frustumPlanes.push(new TFrustumPlane());
			}
			CameraObject.buildViewFrustum(raw.VP, raw.frustumPlanes);
		}
	}

	static right = (raw: TLightObject): vec4_t => {
		return vec4_create(raw.V._00, raw.V._10, raw.V._20);
	}

	static up = (raw: TLightObject): vec4_t => {
		return vec4_create(raw.V._01, raw.V._11, raw.V._21);
	}

	static look = (raw: TLightObject): vec4_t => {
		return vec4_create(raw.V._02, raw.V._12, raw.V._22);
	}
}
