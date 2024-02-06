
///if arm_particles

class particle_sys_t {
	data: particle_data_t;
	speed = 1.0;
	particles: particle_t[];
	ready: bool;
	frame_rate = 24;
	lifetime = 0.0;
	animtime = 0.0;
	time = 0.0;
	spawn_rate = 0.0;
	seed = 0;

	r: particle_data_t;
	gx: f32;
	gy: f32;
	gz: f32;
	alignx: f32;
	aligny: f32;
	alignz: f32;
	dimx: f32;
	dimy: f32;

	count = 0;
	lap = 0;
	lap_time = 0.0;
	m = mat4_identity();

	owner_loc = vec4_create();
	owner_rot = quat_create();
	owner_scale = vec4_create();
}

function particle_sys_create(scene_name: string, ref: particle_ref_t): particle_sys_t {
	let raw = new particle_sys_t();
	raw.seed = ref.seed;
	raw.particles = [];
	raw.ready = false;
	data_get_particle(scene_name, ref.particle, (b: particle_data_t) => {
		raw.data = b;
		raw.r = raw.data;
		raw.gx = 0;
		raw.gy = 0;
		raw.gz = -9.81 * raw.r.weight_gravity;
		raw.alignx = raw.r.object_align_factor[0] / 2;
		raw.aligny = raw.r.object_align_factor[1] / 2;
		raw.alignz = raw.r.object_align_factor[2] / 2;
		raw.lifetime = raw.r.lifetime / raw.frame_rate;
		raw.animtime = (raw.r.frame_end - raw.r.frame_start) / raw.frame_rate;
		raw.spawn_rate = ((raw.r.frame_end - raw.r.frame_start) / raw.r.count) / raw.frame_rate;
		for (let i = 0; i < raw.r.count; ++i) {
			let p = new particle_t();
			p.i = i;
			raw.particles.push(p);
		}
		raw.ready = true;
	});
	return raw;
}

function particle_sys_pause(raw: particle_sys_t) {
	raw.lifetime = 0;
}

function particle_sys_resume(raw: particle_sys_t) {
	raw.lifetime = raw.r.lifetime / raw.frame_rate;
}

function particle_sys_update(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	if (!raw.ready || object == null || raw.speed == 0.0) return;

	// Copy owner world transform but discard scale
	mat4_decompose(owner.base.transform.world, raw.owner_loc, raw.owner_rot, raw.owner_scale);
	object.base.transform.loc = raw.owner_loc;
	object.base.transform.rot = raw.owner_rot;

	// Set particle size per particle system
	object.base.transform.scale = vec4_create(raw.r.particle_size, raw.r.particle_size, raw.r.particle_size, 1);

	transform_build_matrix(object.base.transform);
	transform_build_matrix(owner.base.transform);
	vec4_set_from(object.base.transform.dim, owner.base.transform.dim);

	raw.dimx = object.base.transform.dim.x;
	raw.dimy = object.base.transform.dim.y;

	// Animate
	raw.time += time_real_delta() * raw.speed;
	raw.lap = Math.floor(raw.time / raw.animtime);
	raw.lap_time = raw.time - raw.lap * raw.animtime;
	raw.count = Math.floor(raw.lap_time / raw.spawn_rate);

	particle_sys_update_gpu(raw, object, owner);
}

function particle_sys_get_data(raw: particle_sys_t): mat4_t {
	let hair = raw.r.type == 1;
	raw.m._00 = raw.r.loop ? raw.animtime : -raw.animtime;
	raw.m._01 = hair ? 1 / raw.particles.length : raw.spawn_rate;
	raw.m._02 = hair ? 1 : raw.lifetime;
	raw.m._03 = raw.particles.length;
	raw.m._10 = hair ? 0 : raw.alignx;
	raw.m._11 = hair ? 0 : raw.aligny;
	raw.m._12 = hair ? 0 : raw.alignz;
	raw.m._13 = hair ? 0 : raw.r.factor_random;
	raw.m._20 = hair ? 0 : raw.gx * raw.r.mass;
	raw.m._21 = hair ? 0 : raw.gy * raw.r.mass;
	raw.m._22 = hair ? 0 : raw.gz * raw.r.mass;
	raw.m._23 = hair ? 0 : raw.r.lifetime_random;
	raw.m._30 = 1; // tilesx
	raw.m._31 = 1; // tilesy
	raw.m._32 = 1; // tilesframerate
	raw.m._33 = hair ? 1 : raw.lap_time;
	return raw.m;
}

function particle_sys_update_gpu(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	if (!object.data._instanced) particle_sys_setup_geom(raw, object, owner);
	// GPU particles transform is attached to owner object
}

function particle_sys_rand(max: i32): i32 {
	return Math.floor(Math.random() * max);
}

function particle_sys_setup_geom(raw: particle_sys_t, object: mesh_object_t, owner: mesh_object_t) {
	let instancedData = new Float32Array(raw.particles.length * 3);
	let i = 0;

	let norm_fac = 1 / 32767; // pa.values are not normalized
	let scale_pos_owner = owner.data.scale_pos;
	let scale_pos_particle = object.data.scale_pos;
	let particle_size = raw.r.particle_size;
	let scale_fac = vec4_set_from(vec4_create(), owner.base.transform.scale);
	vec4_mult(scale_fac, scale_pos_owner / (particle_size * scale_pos_particle));

	switch (raw.r.emit_from) {
		case 0: // Vert
			let pa = mesh_data_get_vertex_array(owner.data, 'pos');

			for (let p of raw.particles) {
				let j = Math.floor(particle_sys_fhash(i) * (pa.values.length / pa._size));
				instancedData[i] = pa.values[j * pa._size    ] * norm_fac * scale_fac.x; i++;
				instancedData[i] = pa.values[j * pa._size + 1] * norm_fac * scale_fac.y; i++;
				instancedData[i] = pa.values[j * pa._size + 2] * norm_fac * scale_fac.z; i++;
			}

		case 1: // Face
			let positions = mesh_data_get_vertex_array(owner.data, 'pos').values;

			for (let p of raw.particles) {
				// Choose random index array (there is one per material) and random face
				let ia = owner.data._indices[particle_sys_rand(owner.data._indices.length)];
				let faceIndex = particle_sys_rand(Math.floor(ia.length / 3));

				let i0 = ia[faceIndex * 3 + 0];
				let i1 = ia[faceIndex * 3 + 1];
				let i2 = ia[faceIndex * 3 + 2];

				let v0 = vec3_create(positions[i0 * 4], positions[i0 * 4 + 1], positions[i0 * 4 + 2]);
				let v1 = vec3_create(positions[i1 * 4], positions[i1 * 4 + 1], positions[i1 * 4 + 2]);
				let v2 = vec3_create(positions[i2 * 4], positions[i2 * 4 + 1], positions[i2 * 4 + 2]);

				let pos = particle_sys_random_point_in_triangle(v0, v1, v2);

				instancedData[i] = pos.x * norm_fac * scale_fac.x; i++;
				instancedData[i] = pos.y * norm_fac * scale_fac.y; i++;
				instancedData[i] = pos.z * norm_fac * scale_fac.z; i++;
			}

		case 2: // Volume
			let scaleFactorVolume = vec4_set_from(vec4_create(), object.base.transform.dim);
			vec4_mult(scaleFactorVolume, 0.5 / (particle_size * scale_pos_particle));

			for (let p in raw.particles) {
				instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.x; i++;
				instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.y; i++;
				instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.z; i++;
			}
	}
	mesh_data_setup_inst(object.data, instancedData, 1);
}

function particle_sys_fhash(n: i32): f32 {
	let s = n + 1.0;
	s *= 9301.0 % s;
	s = (s * 9301.0 + 49297.0) % 233280.0;
	return s / 233280.0;
}

function particle_sys_remove(raw: particle_sys_t) {}

function particle_sys_random_point_in_triangle(a: vec3_t, b: vec3_t, c: vec3_t): vec3_t {
	// Generate a random point in a square where (0, 0) <= (x, y) < (1, 1)
	let x = Math.random();
	let y = Math.random();

	if (x + y > 1) {
		// We're in the upper right triangle in the square, mirror to lower left
		x = 1 - x;
		y = 1 - y;
	}

	// Transform the point to the triangle abc
	let u = vec3_sub(b, a);
	let v = vec3_sub(c, a);
	return vec3_add(a, vec3_add(vec3_mult(u, x), vec3_mult(v, y)));
}

class particle_t {
	i: i32;
	x = 0.0;
	y = 0.0;
	z = 0.0;
	camera_dist: f32;
}

///end
