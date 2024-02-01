
class RayCaster {

	static VPInv = Mat4.identity();
	static PInv = Mat4.identity();
	static VInv = Mat4.identity();

	static getRay = (inputX: f32, inputY: f32, camera: TCameraObject): TRay => {
		let start = Vec4.create();
		let end = Vec4.create();
		RayCaster.getDirection(start, end, inputX, inputY, camera);

		// Find direction from start to end
		Vec4.sub(end, start);
		Vec4.normalize(end);
		end.x *= camera.data.far_plane;
		end.y *= camera.data.far_plane;
		end.z *= camera.data.far_plane;

		return Ray.create(start, end);
	}

	static getDirection = (start: TVec4, end: TVec4, inputX: f32, inputY: f32, camera: TCameraObject) => {
		// Get 3D point form screen coords
		// Set two vectors with opposing z values
		start.x = (inputX / App.w()) * 2.0 - 1.0;
		start.y = -((inputY / App.h()) * 2.0 - 1.0);
		start.z = -1.0;
		end.x = start.x;
		end.y = start.y;
		end.z = 1.0;

		Mat4.getInverse(RayCaster.PInv, camera.P);
		Mat4.getInverse(RayCaster.VInv, camera.V);
		Mat4.multmats(RayCaster.VPInv, RayCaster.VInv, RayCaster.PInv);
		Vec4.applyproj(start, RayCaster.VPInv);
		Vec4.applyproj(end, RayCaster.VPInv);
	}

	static boxIntersect = (transform: TransformRaw, inputX: f32, inputY: f32, camera: TCameraObject): TVec4 => {
		let ray = RayCaster.getRay(inputX, inputY, camera);

		let t = transform;
		let c = Vec4.create(Transform.worldx(t), Transform.worldy(t), Transform.worldz(t));
		let s = Vec4.create(t.dim.x, t.dim.y, t.dim.z);
		return Ray.intersectBox(ray, c, s);
	}

	static closestBoxIntersect = (transforms: TransformRaw[], inputX: f32, inputY: f32, camera: TCameraObject): TransformRaw => {
		let intersects: TransformRaw[] = [];

		// Get intersects
		for (let t of transforms) {
			let intersect = RayCaster.boxIntersect(t, inputX, inputY, camera);
			if (intersect != null) intersects.push(t);
		}

		// No intersects
		if (intersects.length == 0) return null;

		// Get closest intersect
		let closest: TransformRaw = null;
		let minDist = Infinity;
		for (let t of intersects) {
			let dist = Vec4.distance(t.loc, camera.base.transform.loc);
			if (dist < minDist) {
				minDist = dist;
				closest = t;
			}
		}

		return closest;
	}

	static planeIntersect = (normal: TVec4, a: TVec4, inputX: f32, inputY: f32, camera: TCameraObject): TVec4 => {
		let ray = RayCaster.getRay(inputX, inputY, camera);

		let plane = new TPlane();
		Plane.set(plane, normal, a);

		return Ray.intersectPlane(ray, plane);
	}
}

class TRay {
	origin: TVec4;
	direction: TVec4;
}

class Ray {

	static create(origin: TVec4 = null, direction: TVec4 = null): TRay {
		let raw = new TRay();
		raw.origin = origin == null ? Vec4.create() : origin;
		raw.direction = direction == null ? Vec4.create() : direction;
		return raw;
	}

	static at = (raw: TRay, t: f32): TVec4 => {
		let result = Vec4.create();
		return Vec4.add(Vec4.mult(Vec4.setFrom(result, raw.direction), t), raw.origin);
	}

	static distanceToPoint = (raw: TRay, point: TVec4): f32 => {
		let v1 = Vec4.create();
		let directionDistance = Vec4.dot(Vec4.subvecs(v1, point, raw.origin), raw.direction);

		// Point behind the ray
		if (directionDistance < 0) {
			return Vec4.distanceTo(raw.origin, point);
		}

		Vec4.add(Vec4.mult(Vec4.setFrom(v1, raw.direction), directionDistance), raw.origin);

		return Vec4.distanceTo(v1, point);
	}

	static intersectsSphere = (raw: TRay, sphereCenter: TVec4, sphereRadius: f32): bool => {
		return Ray.distanceToPoint(raw, sphereCenter) <= sphereRadius;
	}

	static intersectsPlane = (raw: TRay, plane: TPlane): bool => {
		// Check if the ray lies on the plane first
		let distToPoint = Plane.distanceToPoint(plane, raw.origin);
		if (distToPoint == 0) return true;

		let denominator = Vec4.dot(plane.normal, raw.direction);
		if (denominator * distToPoint < 0) return true;

		// Ray origin is behind the plane (and is pointing behind it)
		return false;
	}

	static distanceToPlane = (raw: TRay, plane: TPlane): f32 => {
		let denominator = Vec4.dot(plane.normal, raw.direction);
		if (denominator == 0) {
			// Line is coplanar, return origin
			if (Plane.distanceToPoint(plane, raw.origin) == 0) {
				return 0;
			}

			// Null is preferable to undefined since undefined means.... it is undefined
			return -1;
		}

		let t = -(Vec4.dot(raw.origin, plane.normal) + plane.constant) / denominator;

		// Return if the ray never intersects the plane
		return t >= 0 ? t : -1;
	}

	static intersectPlane = (raw: TRay, plane: TPlane): TVec4 => {
		let t = Ray.distanceToPlane(raw, plane);
		if (t == -1) return null;
		return Ray.at(raw, t);
	}

	static intersectBox = (raw: TRay, center: TVec4, dim: TVec4): TVec4 => {
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

	static intersectTriangle = (raw: TRay, a: TVec4, b: TVec4, c: TVec4, backfaceCulling: bool): TVec4 => {
		// Compute the offset origin, edges, and normal
		let diff = Vec4.create();
		let edge1 = Vec4.create();
		let edge2 = Vec4.create();
		let normal = Vec4.create();

		// from http://www.geometrictools.com/LibMathematics/Intersection/Wm5IntrRay3Triangle3.cpp
		Vec4.subvecs(edge1, b, a);
		Vec4.subvecs(edge2, c, a);
		Vec4.crossvecs(normal, edge1, edge2);

		let DdN = Vec4.dot(raw.direction, normal);
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

		Vec4.subvecs(diff, raw.origin, a);
		let DdQxE2 = sign * Vec4.dot(raw.direction, Vec4.crossvecs(edge2, diff, edge2));

		// b1 < 0, no intersection
		if (DdQxE2 < 0) {
			return null;
		}

		let DdE1xQ = sign * Vec4.dot(raw.direction, Vec4.cross(edge1, diff));

		// b2 < 0, no intersection
		if (DdE1xQ < 0) {
			return null;
		}

		// b1+b2 > 1, no intersection
		if (DdQxE2 + DdE1xQ > DdN) {
			return null;
		}

		// Line intersects triangle, check if ray does.
		let QdN = -sign * Vec4.dot(diff, normal);

		// t < 0, no intersection
		if (QdN < 0) {
			return null;
		}

		// Ray intersects triangle.
		return Ray.at(raw, QdN / DdN);
	}
}

class TPlane {
	normal = Vec4.create(1.0, 0.0, 0.0);
	constant = 0.0;
}

class Plane {
	static distanceToPoint = (raw: TPlane, point: TVec4): f32 => {
		return Vec4.dot(raw.normal, point) + raw.constant;
	}

	static set = (raw: TPlane, normal: TVec4, point: TVec4): TPlane => {
		Vec4.setFrom(raw.normal, normal);
		raw.constant = -Vec4.dot(point, raw.normal);
		return raw;
	}
}
