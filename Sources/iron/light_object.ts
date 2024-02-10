/// <reference path='./vec4.ts'/>
/// <reference path='./mat4.ts'/>

class light_object_t {
	base: object_t;
	data: light_data_t;
	v: mat4_t;
	p: mat4_t;
	vp: mat4_t;
	frustum_planes: frustum_plane_t[];
}

function light_object_create(data: light_data_t): light_object_t {
	let raw = new light_object_t();
	raw.v = mat4_identity();
	raw.vp = mat4_identity();
	raw.base = object_create(false);
	raw.base.ext = raw;
	raw.data = data;

	let type = data.type;
	let fov = data.fov;

	if (type == "sun") {
		raw.p = mat4_ortho(-1, 1, -1, 1, data.near_plane, data.far_plane);
	}
	else if (type == "point" || type == "area") {
		raw.p = mat4_persp(fov, 1, data.near_plane, data.far_plane);
	}
	else if (type == "spot") {
		raw.p = mat4_persp(fov, 1, data.near_plane, data.far_plane);
	}

	scene_lights.push(raw);
	return raw;
}

function light_object_remove(raw: light_object_t) {
	array_remove(scene_lights, raw);
	if (_render_path_light == raw) {
		_render_path_light = null;
	}
	if (_render_path_point == raw) {
		_render_path_point = null;
	}
	else if (_render_path_sun == raw) {
		_render_path_sun = null;
	}

	object_remove_super(raw.base);
}

function light_object_build_mat(raw: light_object_t, camera: camera_object_t) {
	transform_build_matrix(raw.base.transform);
	if (raw.data.type == "sun") { // Cover camera frustum
		mat4_get_inv(raw.v, raw.base.transform.world);
		light_object_update_view_frustum(raw, camera);
	}
	else { // Point, spot, area
		mat4_get_inv(raw.v, raw.base.transform.world);
		light_object_update_view_frustum(raw, camera);
	}
}

function light_object_update_view_frustum(raw: light_object_t, camera: camera_object_t) {
	mat4_mult_mats(raw.vp, raw.p, raw.v);

	// Frustum culling enabled
	if (camera.data.frustum_culling) {
		if (raw.frustum_planes == null) {
			raw.frustum_planes = [];
			for (let i = 0; i < 6; ++i) {
				raw.frustum_planes.push(new frustum_plane_t());
			}
		}
		camera_object_build_view_frustum(raw.vp, raw.frustum_planes);
	}
}

function light_object_right(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m[0], raw.v.m[4], raw.v.m[8]);
}

function light_object_up(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m[1], raw.v.m[5], raw.v.m[9]);
}

function light_object_look(raw: light_object_t): vec4_t {
	return vec4_create(raw.v.m[2], raw.v.m[6], raw.v.m[10]);
}
