/// <reference path='./vec4.ts'/>
/// <reference path='./quat.ts'/>

class camera_object_t {
	base: object_t;
	data: camera_data_t;
	p: mat4_t;
	no_jitter_p = mat4_identity();
	frame = 0;
	v: mat4_t;
	prev_v: mat4_t = null;
	vp: mat4_t;
	frustum_planes: frustum_plane_t[] = null;
	render_target: image_t = null; // Render camera view to texture
	current_face = 0;
}

let camera_object_v = vec4_create();
let camera_object_q = quat_create();
let camera_object_sphere_center = vec4_create();
let camera_object_vcenter = vec4_create();
let camera_object_vup = vec4_create();

function camera_object_create(data: camera_data_t): camera_object_t {
	let raw = new camera_object_t();
	raw.base = object_create();
	raw.base.ext = raw;
	raw.data = data;

	camera_object_build_projection(raw);

	raw.v = mat4_identity();
	raw.vp = mat4_identity();

	if (data.frustum_culling) {
		raw.frustum_planes = [];
		for (let i = 0; i < 6; ++i) {
			raw.frustum_planes.push(new frustum_plane_t());
		}
	}

	scene_cameras.push(raw);
	return raw;
}

function camera_object_build_projection(raw: camera_object_t, screen_aspect: Null<f32> = null) {
	if (raw.data.ortho != null) {
		raw.p = mat4_ortho(raw.data.ortho[0], raw.data.ortho[1], raw.data.ortho[2], raw.data.ortho[3], raw.data.near_plane, raw.data.far_plane);
	}
	else {
		if (screen_aspect == null) {
			screen_aspect = app_w() / app_h();
		}
		let aspect = raw.data.aspect != null ? raw.data.aspect : screen_aspect;
		raw.p = mat4_persp(raw.data.fov, aspect, raw.data.near_plane, raw.data.far_plane);
	}
	mat4_set_from(raw.no_jitter_p, raw.p);
}

function camera_object_remove(raw: camera_object_t) {
	array_remove(scene_cameras, raw);
	object_remove_super(raw.base);
}

function camera_object_render_frame(raw: camera_object_t) {
	camera_object_projection_jitter(raw);
	camera_object_build_matrix(raw);
	render_path_render_frame();
	mat4_set_from(raw.prev_v, raw.v);
}

function camera_object_projection_jitter(raw: camera_object_t) {
	let w = render_path_current_w;
	let h = render_path_current_h;
	mat4_set_from(raw.p, raw.no_jitter_p);
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
	raw.p._20 += x / w;
	raw.p._21 += y / h;
	raw.frame++;
}

function camera_object_build_matrix(raw: camera_object_t) {
	transform_build_matrix(raw.base.transform);

	// Prevent camera matrix scaling
	// TODO: discards position affected by scaled camera parent
	let sc = mat4_get_scale(raw.base.transform.world);
	if (sc.x != 1.0 || sc.y != 1.0 || sc.z != 1.0) {
		vec4_set(camera_object_v, 1.0 / sc.x, 1.0 / sc.y, 1.0 / sc.z);
		mat4_scale(raw.base.transform.world, camera_object_v);
	}

	mat4_get_inv(raw.v, raw.base.transform.world);
	mat4_mult_mats(raw.vp, raw.p, raw.v);

	if (raw.data.frustum_culling) {
		camera_object_build_view_frustum(raw.vp, raw.frustum_planes);
	}

	// First time setting up previous V, prevents first frame flicker
	if (raw.prev_v == null) {
		raw.prev_v = mat4_identity();
		mat4_set_from(raw.prev_v, raw.v);
	}
}

function camera_object_right(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.local._00, raw.base.transform.local._01, raw.base.transform.local._02);
}

function camera_object_up(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.local._10, raw.base.transform.local._11, raw.base.transform.local._12);
}

function camera_object_look(raw: camera_object_t): vec4_t {
	return vec4_create(-raw.base.transform.local._20, -raw.base.transform.local._21, -raw.base.transform.local._22);
}

function camera_object_right_world(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.world._00, raw.base.transform.world._01, raw.base.transform.world._02);
}

function camera_object_up_world(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.world._10, raw.base.transform.world._11, raw.base.transform.world._12);
}

function camera_object_look_world(raw: camera_object_t): vec4_t {
	return vec4_create(-raw.base.transform.world._20, -raw.base.transform.world._21, -raw.base.transform.world._22);
}

function camera_object_build_view_frustum(vp: mat4_t, frustum_planes: frustum_plane_t[]) {
	// Left plane
	frustum_plane_set_components(frustum_planes[0], vp._03 + vp._00, vp._13 + vp._10, vp._23 + vp._20, vp._33 + vp._30);
	// Right plane
	frustum_plane_set_components(frustum_planes[1], vp._03 - vp._00, vp._13 - vp._10, vp._23 - vp._20, vp._33 - vp._30);
	// Top plane
	frustum_plane_set_components(frustum_planes[2], vp._03 - vp._01, vp._13 - vp._11, vp._23 - vp._21, vp._33 - vp._31);
	// Bottom plane
	frustum_plane_set_components(frustum_planes[3], vp._03 + vp._01, vp._13 + vp._11, vp._23 + vp._21, vp._33 + vp._31);
	// Near plane
	frustum_plane_set_components(frustum_planes[4], vp._02, vp._12, vp._22, vp._32);
	// Far plane
	frustum_plane_set_components(frustum_planes[5], vp._03 - vp._02, vp._13 - vp._12, vp._23 - vp._22, vp._33 - vp._32);
	// Normalize planes
	for (let plane of frustum_planes) {
		frustum_plane_normalize(plane);
	}
}

function camera_object_sphere_in_frustum(frustum_planes: frustum_plane_t[], t: transform_t, radius_scale = 1.0, offset_x = 0.0, offset_y = 0.0, offset_z = 0.0): bool {
	// Use scale when radius is changing
	let radius = t.radius * radius_scale;
	for (let plane of frustum_planes) {
		vec4_set(camera_object_sphere_center, transform_world_x(t) + offset_x, transform_world_y(t) + offset_y, transform_world_z(t) + offset_z);
		// Outside the frustum
		if (frustum_plane_dist_to_sphere(plane, camera_object_sphere_center, radius) + radius * 2 < 0) {
			return false;
		}
	}
	return true;
}

class frustum_plane_t {
	normal = vec4_create(1.0, 0.0, 0.0);
	constant = 0.0;
}

function frustum_plane_normalize(raw: frustum_plane_t) {
	let inv_normal_length = 1.0 / vec4_len(raw.normal);
	vec4_mult(raw.normal, inv_normal_length);
	raw.constant *= inv_normal_length;
}

function frustum_plane_dist_to_sphere(raw: frustum_plane_t, sphere_center: vec4_t, sphere_radius: f32): f32 {
	return (vec4_dot(raw.normal, sphere_center) + raw.constant) - sphere_radius;
}

function frustum_plane_set_components(raw: frustum_plane_t, x: f32, y: f32, z: f32, w: f32) {
	vec4_set(raw.normal, x, y, z);
	raw.constant = w;
}
