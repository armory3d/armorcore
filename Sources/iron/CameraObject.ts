/// <reference path='./Vec4.ts'/>
/// <reference path='./Quat.ts'/>

class TCameraObject {
	base: TBaseObject;
	data: TCameraData;
	P: TMat4;
	noJitterP = Mat4.identity();
	frame = 0;
	V: TMat4;
	prevV: TMat4 = null;
	VP: TMat4;
	frustumPlanes: TFrustumPlane[] = null;
	renderTarget: ImageRaw = null; // Render camera view to texture
	currentFace = 0;
}

class CameraObject {
	static temp = Vec4.create();
	static q = Quat.create();
	static sphereCenter = Vec4.create();
	static vcenter = Vec4.create();
	static vup = Vec4.create();

	static create(data: TCameraData): TCameraObject {
		let raw = new TCameraObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;
		raw.data = data;

		CameraObject.buildProjection(raw);

		raw.V = Mat4.identity();
		raw.VP = Mat4.identity();

		if (data.frustum_culling) {
			raw.frustumPlanes = [];
			for (let i = 0; i < 6; ++i) {
				raw.frustumPlanes.push(new TFrustumPlane());
			}
		}

		Scene.cameras.push(raw);
		return raw;
	}

	static buildProjection = (raw: TCameraObject, screenAspect: Null<f32> = null) => {
		if (raw.data.ortho != null) {
			raw.P = Mat4.ortho(raw.data.ortho[0], raw.data.ortho[1], raw.data.ortho[2], raw.data.ortho[3], raw.data.near_plane, raw.data.far_plane);
		}
		else {
			if (screenAspect == null) screenAspect = App.w() / App.h();
			let aspect = raw.data.aspect != null ? raw.data.aspect : screenAspect;
			raw.P = Mat4.persp(raw.data.fov, aspect, raw.data.near_plane, raw.data.far_plane);
		}
		Mat4.setFrom(raw.noJitterP, raw.P);
	}

	static remove = (raw: TCameraObject) => {
		array_remove(Scene.cameras, raw);
		// if (renderTarget != null) renderTarget.unload();

		BaseObject.removeSuper(raw.base);
	}

	static renderFrame = (raw: TCameraObject, g: Graphics4Raw) => {
		CameraObject.projectionJitter(raw);
		CameraObject.buildMatrix(raw);
		RenderPath.renderFrame(g);
		Mat4.setFrom(raw.prevV, raw.V);
	}

	static projectionJitter = (raw: TCameraObject) => {
		let w = RenderPath.currentW;
		let h = RenderPath.currentH;
		Mat4.setFrom(raw.P, raw.noJitterP);
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
		Transform.buildMatrix(raw.base.transform);

		// Prevent camera matrix scaling
		// TODO: discards position affected by scaled camera parent
		let sc = Mat4.getScale(raw.base.transform.world);
		if (sc.x != 1.0 || sc.y != 1.0 || sc.z != 1.0) {
			Vec4.set(CameraObject.temp, 1.0 / sc.x, 1.0 / sc.y, 1.0 / sc.z);
			Mat4.scale(raw.base.transform.world, CameraObject.temp);
		}

		Mat4.getInverse(raw.V, raw.base.transform.world);
		Mat4.multmats(raw.VP, raw.P, raw.V);

		if (raw.data.frustum_culling) {
			CameraObject.buildViewFrustum(raw.VP, raw.frustumPlanes);
		}

		// First time setting up previous V, prevents first frame flicker
		if (raw.prevV == null) {
			raw.prevV = Mat4.identity();
			Mat4.setFrom(raw.prevV, raw.V);
		}
	}

	static right = (raw: TCameraObject): TVec4 => {
		return Vec4.create(raw.base.transform.local._00, raw.base.transform.local._01, raw.base.transform.local._02);
	}

	static up = (raw: TCameraObject): TVec4 => {
		return Vec4.create(raw.base.transform.local._10, raw.base.transform.local._11, raw.base.transform.local._12);
	}

	static look = (raw: TCameraObject): TVec4 => {
		return Vec4.create(-raw.base.transform.local._20, -raw.base.transform.local._21, -raw.base.transform.local._22);
	}

	static rightWorld = (raw: TCameraObject): TVec4 => {
		return Vec4.create(raw.base.transform.world._00, raw.base.transform.world._01, raw.base.transform.world._02);
	}

	static upWorld = (raw: TCameraObject): TVec4 => {
		return Vec4.create(raw.base.transform.world._10, raw.base.transform.world._11, raw.base.transform.world._12);
	}

	static lookWorld = (raw: TCameraObject): TVec4 => {
		return Vec4.create(-raw.base.transform.world._20, -raw.base.transform.world._21, -raw.base.transform.world._22);
	}

	static buildViewFrustum = (VP: TMat4, frustumPlanes: TFrustumPlane[]) => {
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

	static sphereInFrustum = (frustumPlanes: TFrustumPlane[], t: TransformRaw, radiusScale = 1.0, offsetX = 0.0, offsetY = 0.0, offsetZ = 0.0): bool => {
		// Use scale when radius is changing
		let radius = t.radius * radiusScale;
		for (let plane of frustumPlanes) {
			Vec4.set(CameraObject.sphereCenter, Transform.worldx(t) + offsetX, Transform.worldy(t) + offsetY, Transform.worldz(t) + offsetZ);
			// Outside the frustum
			if (FrustumPlane.distanceToSphere(plane, CameraObject.sphereCenter, radius) + radius * 2 < 0) {
				return false;
			}
		}
		return true;
	}
}

class TFrustumPlane {
	normal = Vec4.create(1.0, 0.0, 0.0);
	constant = 0.0;
}

class FrustumPlane {
	static normalize = (raw: TFrustumPlane) => {
		let inverseNormalLength = 1.0 / Vec4.vec4_length(raw.normal);
		Vec4.mult(raw.normal, inverseNormalLength);
		raw.constant *= inverseNormalLength;
	}

	static distanceToSphere = (raw: TFrustumPlane, sphereCenter: TVec4, sphereRadius: f32): f32 => {
		return (Vec4.dot(raw.normal, sphereCenter) + raw.constant) - sphereRadius;
	}

	static setComponents = (raw: TFrustumPlane, x: f32, y: f32, z: f32, w: f32) => {
		Vec4.set(raw.normal, x, y, z);
		raw.constant = w;
	}
}
