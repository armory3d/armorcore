
type camera_object_t = {
	base?: object_t;
	data?: camera_data_t;
	p?: mat4_t;
	no_jitter_p?: mat4_t;
	frame?: i32;
	v?: mat4_t;
	prev_v?: mat4_t;
	vp?: mat4_t;
	frustum_planes?: frustum_plane_t[];
};

let _camera_object_v: vec4_t = vec4_create();
let _camera_object_sphere_center: vec4_t = vec4_create();

function camera_object_create(data: camera_data_t): camera_object_t {
	let raw: camera_object_t = {};
	raw.no_jitter_p = mat4_identity();
	raw.frame = 0;
	raw.base = object_create(false);
	raw.base.ext = raw;
	raw.base.ext_type = "camera_object_t";
	raw.data = data;

	camera_object_build_proj(raw);

	raw.v = mat4_identity();
	raw.vp = mat4_identity();

	if (data.frustum_culling) {
		raw.frustum_planes = [];
		for (let i: i32 = 0; i < 6; ++i) {
			array_push(raw.frustum_planes, frustum_plane_create());
		}
	}

	array_push(scene_cameras, raw);
	return raw;
}

function camera_object_build_proj(raw: camera_object_t, screen_aspect: f32 = -1.0) {
	if (raw.data.ortho != null) {
		raw.p = mat4_ortho(raw.data.ortho[0], raw.data.ortho[1], raw.data.ortho[2], raw.data.ortho[3], raw.data.near_plane, raw.data.far_plane);
	}
	else {
		if (screen_aspect < 0) {
			screen_aspect = app_w() / app_h();
		}
		let aspect: f32 = raw.data.aspect != null ? raw.data.aspect : screen_aspect;
		raw.p = mat4_persp(raw.data.fov, aspect, raw.data.near_plane, raw.data.far_plane);
	}
	mat4_set_from(raw.no_jitter_p, raw.p);
}

function camera_object_remove(raw: camera_object_t) {
	array_remove(scene_cameras, raw);
	object_remove_super(raw.base);
}

function camera_object_render_frame(raw: camera_object_t) {
	camera_object_proj_jitter(raw);
	camera_object_build_mat(raw);
	render_path_render_frame();
	mat4_set_from(raw.prev_v, raw.v);
}

function camera_object_proj_jitter(raw: camera_object_t) {
	let w: i32 = render_path_current_w;
	let h: i32 = render_path_current_h;
	mat4_set_from(raw.p, raw.no_jitter_p);
	let x: f32 = 0.0;
	let y: f32 = 0.0;
	if (raw.frame % 2 == 0) {
		x = 0.25;
		y = 0.25;
	}
	else {
		x = -0.25;
		y = -0.25;
	}
	raw.p.m[8] += x / w;
	raw.p.m[9] += y / h;
	raw.frame++;
}

function camera_object_build_mat(raw: camera_object_t) {
	transform_build_matrix(raw.base.transform);

	// Prevent camera matrix scaling
	// TODO: discards position affected by scaled camera parent
	let sc: vec4_t = mat4_get_scale(raw.base.transform.world);
	if (sc.x != 1.0 || sc.y != 1.0 || sc.z != 1.0) {
		vec4_set(_camera_object_v, 1.0 / sc.x, 1.0 / sc.y, 1.0 / sc.z);
		mat4_scale(raw.base.transform.world, _camera_object_v);
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
	return vec4_create(raw.base.transform.local.m[0], raw.base.transform.local.m[1], raw.base.transform.local.m[2]);
}

function camera_object_up(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.local.m[4], raw.base.transform.local.m[5], raw.base.transform.local.m[6]);
}

function camera_object_look(raw: camera_object_t): vec4_t {
	return vec4_create(-raw.base.transform.local.m[8], -raw.base.transform.local.m[9], -raw.base.transform.local.m[10]);
}

function camera_object_right_world(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.world.m[0], raw.base.transform.world.m[1], raw.base.transform.world.m[2]);
}

function camera_object_up_world(raw: camera_object_t): vec4_t {
	return vec4_create(raw.base.transform.world.m[4], raw.base.transform.world.m[5], raw.base.transform.world.m[6]);
}

function camera_object_look_world(raw: camera_object_t): vec4_t {
	return vec4_create(-raw.base.transform.world.m[8], -raw.base.transform.world.m[9], -raw.base.transform.world.m[10]);
}

function camera_object_build_view_frustum(vp: mat4_t, frustum_planes: frustum_plane_t[]) {
	// Left plane
	frustum_plane_set_components(frustum_planes[0], vp.m[3] + vp.m[0], vp.m[7] + vp.m[4], vp.m[11] + vp.m[8], vp.m[15] + vp.m[12]);
	// Right plane
	frustum_plane_set_components(frustum_planes[1], vp.m[3] - vp.m[0], vp.m[7] - vp.m[4], vp.m[11] - vp.m[8], vp.m[15] - vp.m[12]);
	// Top plane
	frustum_plane_set_components(frustum_planes[2], vp.m[3] - vp.m[1], vp.m[7] - vp.m[5], vp.m[11] - vp.m[9], vp.m[15] - vp.m[13]);
	// Bottom plane
	frustum_plane_set_components(frustum_planes[3], vp.m[3] + vp.m[1], vp.m[7] + vp.m[5], vp.m[11] + vp.m[9], vp.m[15] + vp.m[13]);
	// Near plane
	frustum_plane_set_components(frustum_planes[4], vp.m[2], vp.m[6], vp.m[10], vp.m[14]);
	// Far plane
	frustum_plane_set_components(frustum_planes[5], vp.m[3] - vp.m[2], vp.m[7] - vp.m[6], vp.m[11] - vp.m[10], vp.m[15] - vp.m[14]);
	// Normalize planes
	for (let i: i32 = 0; i < frustum_planes.length; ++i) {
		let plane: frustum_plane_t = frustum_planes[i];
		frustum_plane_normalize(plane);
	}
}

function camera_object_sphere_in_frustum(frustum_planes: frustum_plane_t[], t: transform_t, radius_scale: f32 = 1.0, offset_x: f32 = 0.0, offset_y: f32 = 0.0, offset_z: f32 = 0.0): bool {
	// Use scale when radius is changing
	let radius: f32 = t.radius * radius_scale;
	for (let i: i32 = 0; i < frustum_planes.length; ++i) {
		let plane: frustum_plane_t = frustum_planes[i];
		vec4_set(_camera_object_sphere_center, transform_world_x(t) + offset_x, transform_world_y(t) + offset_y, transform_world_z(t) + offset_z);
		// Outside the frustum
		if (frustum_plane_dist_to_sphere(plane, _camera_object_sphere_center, radius) + radius * 2 < 0) {
			return false;
		}
	}
	return true;
}

type frustum_plane_t = {
	normal?: vec4_t;
	constant?: f32;
};

function frustum_plane_create(): frustum_plane_t {
	let raw: frustum_plane_t = {};
	raw.normal = vec4_create(1.0, 0.0, 0.0);
	raw.constant = 0.0;
	return raw;
}

function frustum_plane_normalize(raw: frustum_plane_t) {
	let inv_normal_length: f32 = 1.0 / vec4_len(raw.normal);
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
