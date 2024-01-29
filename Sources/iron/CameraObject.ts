/// <reference path='./Vec4.ts'/>
/// <reference path='./Quat.ts'/>

class CameraObject extends BaseObject {

	data: TCameraData;
	P: Mat4;
	noJitterP = Mat4.identity();
	frame = 0;
	V: Mat4;
	prevV: Mat4 = null;
	VP: Mat4;
	frustumPlanes: FrustumPlane[] = null;
	renderTarget: Image = null; // Render camera view to texture
	currentFace = 0;

	static temp = new Vec4();
	static q = new Quat();
	static sphereCenter = new Vec4();
	static vcenter = new Vec4();
	static vup = new Vec4();

	constructor(data: TCameraData) {
		super();

		this.data = data;

		this.buildProjection();

		this.V = Mat4.identity();
		this.VP = Mat4.identity();

		if (data.frustum_culling) {
			this.frustumPlanes = [];
			for (let i = 0; i < 6; ++i) this.frustumPlanes.push(new FrustumPlane());
		}

		Scene.cameras.push(this);
	}

	buildProjection = (screenAspect: Null<f32> = null) => {
		if (this.data.ortho != null) {
			this.P = Mat4.ortho(this.data.ortho[0], this.data.ortho[1], this.data.ortho[2], this.data.ortho[3], this.data.near_plane, this.data.far_plane);
		}
		else {
			if (screenAspect == null) screenAspect = App.w() / App.h();
			let aspect = this.data.aspect != null ? this.data.aspect : screenAspect;
			this.P = Mat4.persp(this.data.fov, aspect, this.data.near_plane, this.data.far_plane);
		}
		this.noJitterP.setFrom(this.P);
	}

	override remove = () => {
		array_remove(Scene.cameras, this);
		// if (renderTarget != null) renderTarget.unload();
		this.removeSuper();
	}

	renderFrame = (g: Graphics4) => {
		this.projectionJitter();
		this.buildMatrix();
		RenderPath.renderFrame(g);
		this.prevV.setFrom(this.V);
	}

	projectionJitter = () => {
		let w = RenderPath.currentW;
		let h = RenderPath.currentH;
		this.P.setFrom(this.noJitterP);
		let x = 0.0;
		let y = 0.0;
		if (this.frame % 2 == 0) {
			x = 0.25;
			y = 0.25;
		}
		else {
			x = -0.25;
			y = -0.25;
		}
		this.P._20 += x / w;
		this.P._21 += y / h;
		this.frame++;
	}

	buildMatrix = () => {
		this.transform.buildMatrix();

		// Prevent camera matrix scaling
		// TODO: discards position affected by scaled camera parent
		let sc = this.transform.world.getScale();
		if (sc.x != 1.0 || sc.y != 1.0 || sc.z != 1.0) {
			CameraObject.temp.set(1.0 / sc.x, 1.0 / sc.y, 1.0 / sc.z);
			this.transform.world.scale(CameraObject.temp);
		}

		this.V.getInverse(this.transform.world);
		this.VP.multmats(this.P, this.V);

		if (this.data.frustum_culling) {
			CameraObject.buildViewFrustum(this.VP, this.frustumPlanes);
		}

		// First time setting up previous V, prevents first frame flicker
		if (this.prevV == null) {
			this.prevV = Mat4.identity();
			this.prevV.setFrom(this.V);
		}
	}

	static buildViewFrustum = (VP: Mat4, frustumPlanes: FrustumPlane[]) => {
		// Left plane
		frustumPlanes[0].setComponents(VP._03 + VP._00, VP._13 + VP._10, VP._23 + VP._20, VP._33 + VP._30);
		// Right plane
		frustumPlanes[1].setComponents(VP._03 - VP._00, VP._13 - VP._10, VP._23 - VP._20, VP._33 - VP._30);
		// Top plane
		frustumPlanes[2].setComponents(VP._03 - VP._01, VP._13 - VP._11, VP._23 - VP._21, VP._33 - VP._31);
		// Bottom plane
		frustumPlanes[3].setComponents(VP._03 + VP._01, VP._13 + VP._11, VP._23 + VP._21, VP._33 + VP._31);
		// Near plane
		frustumPlanes[4].setComponents(VP._02, VP._12, VP._22, VP._32);
		// Far plane
		frustumPlanes[5].setComponents(VP._03 - VP._02, VP._13 - VP._12, VP._23 - VP._22, VP._33 - VP._32);
		// Normalize planes
		for (let plane of frustumPlanes) plane.normalize();
	}

	static sphereInFrustum = (frustumPlanes: FrustumPlane[], t: Transform, radiusScale = 1.0, offsetX = 0.0, offsetY = 0.0, offsetZ = 0.0): bool => {
		// Use scale when radius is changing
		let radius = t.radius * radiusScale;
		for (let plane of frustumPlanes) {
			CameraObject.sphereCenter.set(t.worldx() + offsetX, t.worldy() + offsetY, t.worldz() + offsetZ);
			// Outside the frustum
			if (plane.distanceToSphere(CameraObject.sphereCenter, radius) + radius * 2 < 0) {
				return false;
			}
		}
		return true;
	}

	right = (): Vec4 => {
		return new Vec4(this.transform.local._00, this.transform.local._01, this.transform.local._02);
	}

	up = (): Vec4 => {
		return new Vec4(this.transform.local._10, this.transform.local._11, this.transform.local._12);
	}

	look = (): Vec4 => {
		return new Vec4(-this.transform.local._20, -this.transform.local._21, -this.transform.local._22);
	}

	rightWorld = (): Vec4 => {
		return new Vec4(this.transform.world._00, this.transform.world._01, this.transform.world._02);
	}

	upWorld = (): Vec4 => {
		return new Vec4(this.transform.world._10, this.transform.world._11, this.transform.world._12);
	}

	lookWorld = (): Vec4 => {
		return new Vec4(-this.transform.world._20, -this.transform.world._21, -this.transform.world._22);
	}
}

class FrustumPlane {
	normal = new Vec4(1.0, 0.0, 0.0);
	constant = 0.0;

	constructor() {}

	normalize = () => {
		let inverseNormalLength = 1.0 / this.normal.length();
		this.normal.mult(inverseNormalLength);
		this.constant *= inverseNormalLength;
	}

	distanceToSphere = (sphereCenter: Vec4, sphereRadius: f32): f32 => {
		return (this.normal.dot(sphereCenter) + this.constant) - sphereRadius;
	}

	setComponents = (x: f32, y: f32, z: f32, w: f32) => {
		this.normal.set(x, y, z);
		this.constant = w;
	}
}
