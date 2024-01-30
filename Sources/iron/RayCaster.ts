
class RayCaster {

	static VPInv = Mat4.identity();
	static PInv = Mat4.identity();
	static VInv = Mat4.identity();

	static getRay = (inputX: f32, inputY: f32, camera: CameraObject): TRay => {
		let start = new Vec4();
		let end = new Vec4();
		RayCaster.getDirection(start, end, inputX, inputY, camera);

		// Find direction from start to end
		end.sub(start);
		end.normalize();
		end.x *= camera.data.far_plane;
		end.y *= camera.data.far_plane;
		end.z *= camera.data.far_plane;

		return Ray.create(start, end);
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
		return Ray.intersectBox(ray, c, s);
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

		let plane = new TPlane();
		Plane.set(plane, normal, a);

		return Ray.intersectPlane(ray, plane);
	}
}

class TRay {
	origin: Vec4;
	direction: Vec4;
}

class Ray {

	static create(origin: Vec4 = null, direction: Vec4 = null): TRay {
		let raw = new TRay();
		raw.origin = origin == null ? new Vec4() : origin;
		raw.direction = direction == null ? new Vec4() : direction;
		return raw;
	}

	static at = (raw: TRay, t: f32): Vec4 => {
		let result = new Vec4();
		return result.setFrom(raw.direction).mult(t).add(raw.origin);
	}

	static distanceToPoint = (raw: TRay, point: Vec4): f32 => {
		let v1 = new Vec4();
		let directionDistance = v1.subvecs(point, raw.origin).dot(raw.direction);

		// Point behind the ray
		if (directionDistance < 0) {
			return raw.origin.distanceTo(point);
		}

		v1.setFrom(raw.direction).mult(directionDistance).add(raw.origin);

		return v1.distanceTo(point);
	}

	static intersectsSphere = (raw: TRay, sphereCenter: Vec4, sphereRadius: f32): bool => {
		return Ray.distanceToPoint(raw, sphereCenter) <= sphereRadius;
	}

	static intersectsPlane = (raw: TRay, plane: TPlane): bool => {
		// Check if the ray lies on the plane first
		let distToPoint = Plane.distanceToPoint(plane, raw.origin);
		if (distToPoint == 0) return true;

		let denominator = plane.normal.dot(raw.direction);
		if (denominator * distToPoint < 0) return true;

		// Ray origin is behind the plane (and is pointing behind it)
		return false;
	}

	static distanceToPlane = (raw: TRay, plane: TPlane): f32 => {
		let denominator = plane.normal.dot(raw.direction);
		if (denominator == 0) {
			// Line is coplanar, return origin
			if (Plane.distanceToPoint(plane, raw.origin) == 0) {
				return 0;
			}

			// Null is preferable to undefined since undefined means.... it is undefined
			return -1;
		}

		let t = -(raw.origin.dot(plane.normal) + plane.constant) / denominator;

		// Return if the ray never intersects the plane
		return t >= 0 ? t : -1;
	}

	static intersectPlane = (raw: TRay, plane: TPlane): Vec4 => {
		let t = Ray.distanceToPlane(raw, plane);
		if (t == -1) return null;
		return Ray.at(raw, t);
	}

	static intersectBox = (raw: TRay, center: Vec4, dim: Vec4): Vec4 => {
		// http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
		let tmin, tmax, tymin, tymax, tzmin, tzmax;

		let halfX = dim.x / 2;
		let halfY = dim.y / 2;
		let halfZ = dim.z / 2;
		let boxMinX = center.x - halfX;
		let boxMinY = center.y - halfY;
		let boxMinZ = center.z - halfZ;
		let boxMaxX = center.x + halfX;
		let boxMaxY = center.y + halfY;
		let boxMaxZ = center.z + halfZ;

		let invdirx = 1 / raw.direction.x;
		let	invdiry = 1 / raw.direction.y;
		let invdirz = 1 / raw.direction.z;

		let origin = raw.origin;

		if (invdirx >= 0) {
			tmin = (boxMinX - origin.x) * invdirx;
			tmax = (boxMaxX - origin.x) * invdirx;
		}
		else {
			tmin = (boxMaxX - origin.x) * invdirx;
			tmax = (boxMinX - origin.x) * invdirx;
		}

		if (invdiry >= 0) {
			tymin = (boxMinY - origin.y) * invdiry;
			tymax = (boxMaxY - origin.y) * invdiry;
		}
		else {
			tymin = (boxMaxY - origin.y) * invdiry;
			tymax = (boxMinY - origin.y) * invdiry;
		}

		if ((tmin > tymax) || (tymin > tmax)) return null;

		// These lines also handle the case where tmin or tmax is NaN
		// (result of 0 * Infinity). x !== x returns true if x is NaN
		if (tymin > tmin || tmin != tmin) tmin = tymin;
		if (tymax < tmax || tmax != tmax) tmax = tymax;

		if (invdirz >= 0) {
			tzmin = (boxMinZ - origin.z) * invdirz;
			tzmax = (boxMaxZ - origin.z) * invdirz;
		}
		else {
			tzmin = (boxMaxZ - origin.z) * invdirz;
			tzmax = (boxMinZ - origin.z) * invdirz;
		}

		if ((tmin > tzmax) || (tzmin > tmax)) return null;
		if (tzmin > tmin || tmin != tmin ) tmin = tzmin;
		if (tzmax < tmax || tmax != tmax ) tmax = tzmax;

		// Return point closest to the ray (positive side)
		if (tmax < 0) return null;

		return Ray.at(raw, tmin >= 0 ? tmin : tmax);
	}

	static intersectTriangle = (raw: TRay, a: Vec4, b: Vec4, c: Vec4, backfaceCulling: bool): Vec4 => {
		// Compute the offset origin, edges, and normal
		let diff = new Vec4();
		let edge1 = new Vec4();
		let edge2 = new Vec4();
		let normal = new Vec4();

		// from http://www.geometrictools.com/LibMathematics/Intersection/Wm5IntrRay3Triangle3.cpp
		edge1.subvecs(b, a);
		edge2.subvecs(c, a);
		normal.crossvecs(edge1, edge2);

		let DdN = raw.direction.dot(normal);
		let sign;

		if (DdN > 0) {
			if (backfaceCulling) return null;
			sign = 1;
		}
		else if (DdN < 0) {
			sign = -1;
			DdN = -DdN;
		}
		else {
			return null;
		}

		diff.subvecs(raw.origin, a);
		let DdQxE2 = sign * raw.direction.dot(edge2.crossvecs(diff, edge2));

		// b1 < 0, no intersection
		if (DdQxE2 < 0) {
			return null;
		}

		let DdE1xQ = sign * raw.direction.dot(edge1.cross(diff));

		// b2 < 0, no intersection
		if (DdE1xQ < 0) {
			return null;
		}

		// b1+b2 > 1, no intersection
		if (DdQxE2 + DdE1xQ > DdN) {
			return null;
		}

		// Line intersects triangle, check if ray does.
		let QdN = -sign * diff.dot(normal);

		// t < 0, no intersection
		if (QdN < 0) {
			return null;
		}

		// Ray intersects triangle.
		return Ray.at(raw, QdN / DdN);
	}
}

class TPlane {
	normal = new Vec4(1.0, 0.0, 0.0);
	constant = 0.0;
}

class Plane {
	static distanceToPoint = (raw: TPlane, point: Vec4): f32 => {
		return raw.normal.dot(point) + raw.constant;
	}

	static set = (raw: TPlane, normal: Vec4, point: Vec4): TPlane => {
		raw.normal.setFrom(normal);
		raw.constant = -point.dot(raw.normal);
		return raw;
	}
}
