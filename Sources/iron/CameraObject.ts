/// <reference path='./vec4.ts'/>
/// <reference path='./quat.ts'/>

class TCameraObject {
	base: TBaseObject;
	data: camera_data_t;
	P: mat4_t;
	noJitterP = mat4_identity();
	frame = 0;
	V: mat4_t;
	prevV: mat4_t = null;
	VP: mat4_t;
	frustumPlanes: TFrustumPlane[] = null;
	renderTarget: image_t = null; // Render camera view to texture
	currentFace = 0;
}

class CameraObject {
	static temp = vec4_create();
	static q = quat_create();
	static sphereCenter = vec4_create();
	static vcenter = vec4_create();
	static vup = vec4_create();

	static create(data: camera_data_t): TCameraObject {
		let raw = new TCameraObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;
		raw.data = data;

		CameraObject.buildProjection(raw);

		raw.V = mat4_identity();
		raw.VP = mat4_identity();

		if (data.frustum_culling) {
			raw.frustumPlanes = [];
			for (let i = 0; i < 6; ++i) {
				raw.frustumPlanes.push(new TFrustumPlane());
			}
		}

		scene_cameras.push(raw);
		return raw;
	}

	static buildProjection = (raw: TCameraObject, screenAspect: Null<f32> = null) => {
		if (raw.data.ortho != null) {
			raw.P = mat4_ortho(raw.data.ortho[0], raw.data.ortho[1], raw.data.ortho[2], raw.data.ortho[3], raw.data.near_plane, raw.data.far_plane);
		}
		else {
			if (screenAspect == null) screenAspect = App.w() / App.h();
			let aspect = raw.data.aspect != null ? raw.data.aspect : screenAspect;
			raw.P = mat4_persp(raw.data.fov, aspect, raw.data.near_plane, raw.data.far_plane);
		}
		mat4_set_from(raw.noJitterP, raw.P);
	}

	static remove = (raw: TCameraObject) => {
		array_remove(scene_cameras, raw);
		// if (renderTarget != null) renderTarget.unload();

		BaseObject.removeSuper(raw.base);
	}

	static renderFrame = (raw: TCameraObject, g: g4_t) => {
		CameraObject.projectionJitter(raw);
		CameraObject.buildMatrix(raw);
		render_path_render_frame(g);
		mat4_set_from(raw.prevV, raw.V);
	}

	static projectionJitter = (raw: TCameraObject) => {
		let w = render_path_current_w;
		let h = render_path_current_h;
		mat4_set_from(raw.P, raw.noJitterP);
		let x = 0.0;
		let y = 0.0;
		if (raw.frame % 2 == 0) {
			x = 0.25;
			y = 0.25;
		}
		else {
			x = -0.25;
			y = -0.25;
		}
		raw.P._20 += x / w;
		raw.P._21 += y / h;
		raw.frame++;
	}

	static buildMatrix = (raw: TCameraObject) => {
		transform_build_matrix(raw.base.transform);

		// Prevent camera matrix scaling
		// TODO: discards position affected by scaled camera parent
		let sc = mat4_get_scale(raw.base.transform.world);
		if (sc.x != 1.0 || sc.y != 1.0 || sc.z != 1.0) {
			vec4_set(CameraObject.temp, 1.0 / sc.x, 1.0 / sc.y, 1.0 / sc.z);
			mat4_scale(raw.base.transform.world, CameraObject.temp);
		}

		mat4_get_inv(raw.V, raw.base.transform.world);
		mat4_mult_mats(raw.VP, raw.P, raw.V);

		if (raw.data.frustum_culling) {
			CameraObject.buildViewFrustum(raw.VP, raw.frustumPlanes);
		}

		// First time setting up previous V, prevents first frame flicker
		if (raw.prevV == null) {
			raw.prevV = mat4_identity();
			mat4_set_from(raw.prevV, raw.V);
		}
	}

	static right = (raw: TCameraObject): vec4_t => {
		return vec4_create(raw.base.transform.local._00, raw.base.transform.local._01, raw.base.transform.local._02);
	}

	static up = (raw: TCameraObject): vec4_t => {
		return vec4_create(raw.base.transform.local._10, raw.base.transform.local._11, raw.base.transform.local._12);
	}

	static look = (raw: TCameraObject): vec4_t => {
		return vec4_create(-raw.base.transform.local._20, -raw.base.transform.local._21, -raw.base.transform.local._22);
	}

	static rightWorld = (raw: TCameraObject): vec4_t => {
		return vec4_create(raw.base.transform.world._00, raw.base.transform.world._01, raw.base.transform.world._02);
	}

	static upWorld = (raw: TCameraObject): vec4_t => {
		return vec4_create(raw.base.transform.world._10, raw.base.transform.world._11, raw.base.transform.world._12);
	}

	static lookWorld = (raw: TCameraObject): vec4_t => {
		return vec4_create(-raw.base.transform.world._20, -raw.base.transform.world._21, -raw.base.transform.world._22);
	}

	static buildViewFrustum = (VP: mat4_t, frustumPlanes: TFrustumPlane[]) => {
		// Left plane
		FrustumPlane.setComponents(frustumPlanes[0], VP._03 + VP._00, VP._13 + VP._10, VP._23 + VP._20, VP._33 + VP._30);
		// Right plane
		FrustumPlane.setComponents(frustumPlanes[1], VP._03 - VP._00, VP._13 - VP._10, VP._23 - VP._20, VP._33 - VP._30);
		// Top plane
		FrustumPlane.setComponents(frustumPlanes[2], VP._03 - VP._01, VP._13 - VP._11, VP._23 - VP._21, VP._33 - VP._31);
		// Bottom plane
		FrustumPlane.setComponents(frustumPlanes[3], VP._03 + VP._01, VP._13 + VP._11, VP._23 + VP._21, VP._33 + VP._31);
		// Near plane
		FrustumPlane.setComponents(frustumPlanes[4], VP._02, VP._12, VP._22, VP._32);
		// Far plane
		FrustumPlane.setComponents(frustumPlanes[5], VP._03 - VP._02, VP._13 - VP._12, VP._23 - VP._22, VP._33 - VP._32);
		// Normalize planes
		for (let plane of frustumPlanes) FrustumPlane.normalize(plane);
	}

	static sphereInFrustum = (frustumPlanes: TFrustumPlane[], t: transform_t, radiusScale = 1.0, offsetX = 0.0, offsetY = 0.0, offsetZ = 0.0): bool => {
		// Use scale when radius is changing
		let radius = t.radius * radiusScale;
		for (let plane of frustumPlanes) {
			vec4_set(CameraObject.sphereCenter, transform_world_x(t) + offsetX, transform_world_y(t) + offsetY, transform_world_z(t) + offsetZ);
			// Outside the frustum
			if (FrustumPlane.distanceToSphere(plane, CameraObject.sphereCenter, radius) + radius * 2 < 0) {
				return false;
			}
		}
		return true;
	}
}

class TFrustumPlane {
	normal = vec4_create(1.0, 0.0, 0.0);
	constant = 0.0;
}

class FrustumPlane {
	static normalize = (raw: TFrustumPlane) => {
		let inverseNormalLength = 1.0 / vec4_len(raw.normal);
		vec4_mult(raw.normal, inverseNormalLength);
		raw.constant *= inverseNormalLength;
	}

	static distanceToSphere = (raw: TFrustumPlane, sphereCenter: vec4_t, sphereRadius: f32): f32 => {
		return (vec4_dot(raw.normal, sphereCenter) + raw.constant) - sphereRadius;
	}

	static setComponents = (raw: TFrustumPlane, x: f32, y: f32, z: f32, w: f32) => {
		vec4_set(raw.normal, x, y, z);
		raw.constant = w;
	}
}
