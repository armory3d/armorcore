
class RayCaster {

	static VPInv = Mat4.identity();
	static PInv = Mat4.identity();
	static VInv = Mat4.identity();

	static getRay = (inputX: f32, inputY: f32, camera: CameraObject): Ray => {
		let start = new Vec4();
		let end = new Vec4();
		RayCaster.getDirection(start, end, inputX, inputY, camera);

		// Find direction from start to end
		end.sub(start);
		end.normalize();
		end.x *= camera.data.raw.far_plane;
		end.y *= camera.data.raw.far_plane;
		end.z *= camera.data.raw.far_plane;

		return new Ray(start, end);
	}

	static getDirection = (start: Vec4, end: Vec4, inputX: f32, inputY: f32, camera: CameraObject) => {
		// Get 3D point form screen coords
		// Set two vectors with opposing z values
		start.x = (inputX / App.w()) * 2.0 - 1.0;
		start.y = -((inputY / App.h()) * 2.0 - 1.0);
		start.z = -1.0;
		end.x = start.x;
		end.y = start.y;
		end.z = 1.0;

		RayCaster.PInv.getInverse(camera.P);
		RayCaster.VInv.getInverse(camera.V);
		RayCaster.VPInv.multmats(RayCaster.VInv, RayCaster.PInv);
		start.applyproj(RayCaster.VPInv);
		end.applyproj(RayCaster.VPInv);
	}

	static boxIntersect = (transform: Transform, inputX: f32, inputY: f32, camera: CameraObject): Vec4 => {
		let ray = RayCaster.getRay(inputX, inputY, camera);

		let t = transform;
		let c = new Vec4(t.worldx(), t.worldy(), t.worldz());
		let s = new Vec4(t.dim.x, t.dim.y, t.dim.z);
		return ray.intersectBox(c, s);
	}

	static closestBoxIntersect = (transforms: Transform[], inputX: f32, inputY: f32, camera: CameraObject): Transform => {
		let intersects: Transform[] = [];

		// Get intersects
		for (let t of transforms) {
			let intersect = RayCaster.boxIntersect(t, inputX, inputY, camera);
			if (intersect != null) intersects.push(t);
		}

		// No intersects
		if (intersects.length == 0) return null;

		// Get closest intersect
		let closest: Transform = null;
		let minDist = Infinity;
		for (let t of intersects) {
			let dist = Vec4.distance(t.loc, camera.transform.loc);
			if (dist < minDist) {
				minDist = dist;
				closest = t;
			}
		}

		return closest;
	}

	static planeIntersect = (normal: Vec4, a: Vec4, inputX: f32, inputY: f32, camera: CameraObject): Vec4 => {
		let ray = RayCaster.getRay(inputX, inputY, camera);

		let plane = new Plane();
		plane.set(normal, a);

		return ray.intersectPlane(plane);
	}
}
