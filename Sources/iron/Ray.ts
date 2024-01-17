
class Ray {

	origin: Vec4;
	direction: Vec4;

	constructor(origin: Vec4 = null, direction: Vec4 = null) {
		this.origin = origin == null ? new Vec4() : origin;
		this.direction = direction == null ? new Vec4() : direction;
	}

	at = (t: f32): Vec4 => {
		let result = new Vec4();
		return result.setFrom(this.direction).mult(t).add(this.origin);
	}

	distanceToPoint = (point: Vec4): f32 => {
		let v1 = new Vec4();
		let directionDistance = v1.subvecs(point, this.origin).dot(this.direction);

		// Point behind the ray
		if (directionDistance < 0) {
			return this.origin.distanceTo(point);
		}

		v1.setFrom(this.direction).mult(directionDistance).add(this.origin);

		return v1.distanceTo(point);
	}

	intersectsSphere = (sphereCenter: Vec4, sphereRadius: f32): bool => {
		return this.distanceToPoint(sphereCenter) <= sphereRadius;
	}

	intersectsPlane = (plane: Plane): bool => {
		// Check if the ray lies on the plane first
		let distToPoint = plane.distanceToPoint(this.origin);
		if (distToPoint == 0) return true;

		let denominator = plane.normal.dot(this.direction);
		if (denominator * distToPoint < 0) return true;

		// Ray origin is behind the plane (and is pointing behind it)
		return false;
	}

	distanceToPlane = (plane: Plane): f32 => {
		let denominator = plane.normal.dot(this.direction);
		if (denominator == 0) {
			// Line is coplanar, return origin
			if (plane.distanceToPoint(this.origin) == 0) {
				return 0;
			}

			// Null is preferable to undefined since undefined means.... it is undefined
			return -1;
		}

		let t = -(this.origin.dot(plane.normal) + plane.constant) / denominator;

		// Return if the ray never intersects the plane
		return t >= 0 ? t : -1;
	}

	intersectPlane = (plane: Plane): Vec4 => {
		let t = this.distanceToPlane(plane);
		if (t == -1) return null;
		return this.at(t);
	}

	intersectBox = (center: Vec4, dim: Vec4): Vec4 => {
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

		let invdirx = 1 / this.direction.x;
		let	invdiry = 1 / this.direction.y;
		let invdirz = 1 / this.direction.z;

		let origin = this.origin;

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

		return this.at(tmin >= 0 ? tmin : tmax);
	}

	intersectTriangle = (a: Vec4, b: Vec4, c: Vec4, backfaceCulling: bool): Vec4 => {
		// Compute the offset origin, edges, and normal
		let diff = new Vec4();
		let edge1 = new Vec4();
		let edge2 = new Vec4();
		let normal = new Vec4();

		// from http://www.geometrictools.com/LibMathematics/Intersection/Wm5IntrRay3Triangle3.cpp
		edge1.subvecs(b, a);
		edge2.subvecs(c, a);
		normal.crossvecs(edge1, edge2);

		let DdN = this.direction.dot(normal);
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

		diff.subvecs(this.origin, a);
		let DdQxE2 = sign * this.direction.dot(edge2.crossvecs(diff, edge2));

		// b1 < 0, no intersection
		if (DdQxE2 < 0) {
			return null;
		}

		let DdE1xQ = sign * this.direction.dot(edge1.cross(diff));

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
		return this.at(QdN / DdN);
	}
}

class Plane {
	normal = new Vec4(1.0, 0.0, 0.0);
	constant = 0.0;

	constructor() {}

	distanceToPoint = (point: Vec4): f32 => {
		return this.normal.dot(point) + this.constant;
	}

	set = (normal: Vec4, point: Vec4): Plane => {
		this.normal.setFrom(normal);
		this.constant = -point.dot(this.normal);
		return this;
	}
}
