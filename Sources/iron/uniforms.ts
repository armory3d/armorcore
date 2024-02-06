/// <reference path='./vec4.ts'/>

let _uniforms_mat = mat4_identity();
let _uniforms_mat2 = mat4_identity();
let _uniforms_mat3 = mat3_identity();
let _uniforms_vec = vec4_create();
let _uniforms_vec2 = vec4_create();
let _uniforms_quat = quat_create();

let uniforms_tex_links: ((o: object_t, md: material_data_t, s: string)=>image_t)[] = null;
let uniforms_mat4_links: ((o: object_t, md: material_data_t, s: string)=>mat4_t)[] = null;
let uniforms_vec4_links: ((o: object_t, md: material_data_t, s: string)=>vec4_t)[] = null;
let uniforms_vec3_links: ((o: object_t, md: material_data_t, s: string)=>vec4_t)[] = null;
let uniforms_vec2_links: ((o: object_t, md: material_data_t, s: string)=>vec4_t)[] = null;
let uniforms_f32_links: ((o: object_t, md: material_data_t, s: string)=>Null<f32>)[] = null;
let uniforms_f32_array_links: ((o: object_t, md: material_data_t, s: string)=>Float32Array)[] = null;
let uniforms_i32_links: ((o: object_t, md: material_data_t, s: string)=>Null<i32>)[] = null;
let uniforms_pos_unpack: Null<f32> = null;
let uniforms_tex_unpack: Null<f32> = null;

function uniforms_set_context_consts(g: g4_t, context: shader_context_t, bind_params: string[]) {
	if (context.constants != null) {
		for (let i = 0; i < context.constants.length; ++i) {
			let c = context.constants[i];
			uniforms_set_context_const(g, context._constants[i], c);
		}
	}

	// Texture context constants
	if (bind_params != null) { // Bind targets
		for (let i = 0; i < Math.floor(bind_params.length / 2); ++i) {
			let pos = i * 2; // bind params = [texture, samplerID]
			let rtID = bind_params[pos];
			let samplerID = bind_params[pos + 1];
			let attachDepth = false; // Attach texture depth if '_' is prepended
			let char = rtID.charAt(0);
			if (char == "_") {
				attachDepth = true;
				rtID = rtID.substr(1);
			}
			let rt = attachDepth ?render_path_depth_to_render_target.get(rtID) :render_path_render_targets.get(rtID);
			uniforms_bind_render_target(g, rt, context, samplerID, attachDepth);
		}
	}

	// Texture links
	if (context.texture_units != null) {
		for (let j = 0; j < context.texture_units.length; ++j) {
			let tulink = context.texture_units[j].link;
			if (tulink == null) continue;

			if (tulink.charAt(0) == "$") { // Link to embedded data
				g4_set_tex(context._tex_units[j], scene_embedded.get(tulink.substr(1)));
				if (tulink.endsWith(".raw")) { // Raw 3D texture
					g4_set_tex_3d_params(context._tex_units[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
				}
				else { // 2D texture
					g4_set_tex_params(context._tex_units[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
				}
			}
			else {
				switch (tulink) {
					case "_envmapRadiance": {
						let w = scene_world;
						if (w != null) {
							g4_set_tex(context._tex_units[j], w._radiance);
							g4_set_tex_params(context._tex_units[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
						}
						break;
					}
					case "_envmap": {
						let w = scene_world;
						if (w != null) {
							g4_set_tex(context._tex_units[j], w._envmap);
							g4_set_tex_params(context._tex_units[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
						}
						break;
					}
				}
			}
		}
	}
}

function uniforms_set_obj_consts(g: g4_t, context: shader_context_t, object: object_t) {
	if (context.constants != null) {
		for (let i = 0; i < context.constants.length; ++i) {
			let c = context.constants[i];
			uniforms_set_obj_const(g, object, context._constants[i], c);
		}
	}

	// Texture object constants
	// External
	if (uniforms_tex_links != null) {
		if (context.texture_units != null) {
			for (let j = 0; j < context.texture_units.length; ++j) {
				let tu = context.texture_units[j];
				if (tu.link == null) continue;
				let tuAddrU = uniforms_get_tex_addressing(tu.addressing_u);
				let tuAddrV = uniforms_get_tex_addressing(tu.addressing_v);
				let tuFilterMin = uniforms_get_tex_filter(tu.filter_min);
				let tuFilterMag = uniforms_get_tex_filter(tu.filter_mag);
				let tuMipMapFilter = uniforms_get_mip_map_filter(tu.mipmap_filter);

				for (let f of uniforms_tex_links) {
					let image = f(object, current_material(object), tu.link);
					if (image != null) {
						tu.link.endsWith("_depth") ?
							g4_set_tex_depth(context._tex_units[j], image) :
							g4_set_tex(context._tex_units[j], image);
						g4_set_tex_params(context._tex_units[j], tuAddrU, tuAddrV, tuFilterMin, tuFilterMag, tuMipMapFilter);
						break;
					}
				}
			}
		}
	}
}

function uniforms_bind_render_target(g: g4_t, rt: render_target_t, context: shader_context_t, sampler_id: string, attach_depth: bool) {
	if (rt != null) {
		let tus = context.texture_units;

		for (let j = 0; j < tus.length; ++j) { // Set texture
			if (sampler_id == tus[j].name) {
				let is_image = tus[j].is_image != null && tus[j].is_image;
				let params_set = false;

				if (rt.depth > 1) { // sampler3D
					g4_set_tex_3d_params(context._tex_units[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.AnisotropicFilter, MipMapFilter.LinearMipFilter);
					params_set = true;
				}

				if (is_image) {
					g4_set_image_tex(context._tex_units[j], rt.image); // image2D/3D
					// Multiple voxel volumes, always set params
					g4_set_tex_3d_params(context._tex_units[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.PointFilter, MipMapFilter.LinearMipFilter);
					params_set = true;
				}
				else {
					if (attach_depth) g4_set_tex_depth(context._tex_units[j], rt.image); // sampler2D
					else g4_set_tex(context._tex_units[j], rt.image); // sampler2D
				}

				if (!params_set && rt.mipmaps != null && rt.mipmaps == true && !is_image) {
					g4_set_tex_params(context._tex_units[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
					params_set = true;
				}

				if (!params_set) {
					if (rt.name.startsWith("bloom")) {
						// Use bilinear filter for bloom mips to get correct blur
						g4_set_tex_params(context._tex_units[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
						params_set = true;
					}
					if (attach_depth) {
						g4_set_tex_params(context._tex_units[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.PointFilter, TextureFilter.PointFilter, MipMapFilter.NoMipFilter);
						params_set = true;
					}
				}

				if (!params_set) {
					// No filtering when sampling render targets
					let oc = context._override_context;
					let allowParams = oc == null || oc.shared_sampler == null || oc.shared_sampler == sampler_id;
					if (allowParams) {
						let addressing = (oc != null && oc.addressing == "repeat") ? TextureAddressing.Repeat : TextureAddressing.Clamp;
						let filter = (oc != null && oc.filter == "point") ? TextureFilter.PointFilter : TextureFilter.LinearFilter;
						g4_set_tex_params(context._tex_units[j], addressing, addressing, filter, filter, MipMapFilter.NoMipFilter);
					}
					params_set = true;
				}
			}
		}
	}
}

function uniforms_set_context_const(g: g4_t, location: kinc_const_loc_t, c: shader_const_t): bool {
	if (c.link == null) return true;

	let camera = scene_camera;
	let light =_render_path_light;

	if (c.type == "mat4") {
		let m: mat4_t = null;
		switch (c.link) {
			case "_viewMatrix": {
				m = camera.v;
				break;
			}
			case "_projectionMatrix": {
				m = camera.p;
				break;
			}
			case "_inverseProjectionMatrix": {
				mat4_get_inv(_uniforms_mat, camera.p);
				m = _uniforms_mat;
				break;
			}
			case "_viewProjectionMatrix": {
				m = camera.vp;
				break;
			}
			case "_inverseViewProjectionMatrix": {
				mat4_set_from(_uniforms_mat, camera.v);
				mat4_mult_mat(_uniforms_mat, camera.p);
				mat4_get_inv(_uniforms_mat, _uniforms_mat);
				m = _uniforms_mat;
				break;
			}
			case "_skydomeMatrix": {
				let tr = camera.base.transform;
				vec4_set(_uniforms_vec, transform_world_x(tr), transform_world_y(tr), transform_world_z(tr) - 3.5); // Sky
				let bounds = camera.data.far_plane * 0.95;
				vec4_set(_uniforms_vec2, bounds, bounds, bounds);
				mat4_compose(_uniforms_mat, _uniforms_vec, _uniforms_quat, _uniforms_vec2);
				mat4_mult_mat(_uniforms_mat, camera.v);
				mat4_mult_mat(_uniforms_mat, camera.p);
				m = _uniforms_mat;
				break;
			}
			default: // Unknown uniform
				return false;
		}

		g4_set_mat(location, m != null ? m : mat4_identity());
		return true;
	}
	else if (c.type == "vec4") {
		let v: vec4_t = null;
		vec4_set(_uniforms_vec, 0, 0, 0, 0);
		switch (c.link) {
			default:
				return false;
		}

		if (v != null) {
			g4_set_float4(location, v.x, v.y, v.z, v.w);
		}
		else {
			g4_set_float4(location, 0, 0, 0, 0);
		}
		return true;
	}
	else if (c.type == "vec3") {
		let v: vec4_t = null;
		vec4_set(_uniforms_vec, 0, 0, 0);
		switch (c.link) {
			case "_lightDirection": {
				if (light != null) {
					_uniforms_vec = vec4_normalize(light_object_look(light));
					v = _uniforms_vec;
					break;
				}
			}
			case "_pointPosition": {
				let point =_render_path_point;
				if (point != null) {
					vec4_set(_uniforms_vec, transform_world_x(point.base.transform), transform_world_y(point.base.transform), transform_world_z(point.base.transform));
					v = _uniforms_vec;
					break;
				}
			}
			case "_pointColor": {
				let point =_render_path_point;
				if (point != null) {
					let str = point.base.visible ? point.data.strength : 0.0;
					vec4_set(_uniforms_vec, point.data.color[0] * str, point.data.color[1] * str, point.data.color[2] * str);
					v = _uniforms_vec;
					break;
				}
			}
			case "_lightArea0": {
				if (light != null && light.data.size != null) {
					let f2: f32 = 0.5;
					let sx: f32 = light.data.size * f2;
					let sy: f32 = light.data.size_y * f2;
					vec4_set(_uniforms_vec, -sx, sy, 0.0);
					vec4_apply_mat(_uniforms_vec, light.base.transform.world);
					v = _uniforms_vec;
					break;
				}
			}
			case "_lightArea1": {
				if (light != null && light.data.size != null) {
					let f2: f32 = 0.5;
					let sx: f32 = light.data.size * f2;
					let sy: f32 = light.data.size_y * f2;
					vec4_set(_uniforms_vec, sx, sy, 0.0);
					vec4_apply_mat(_uniforms_vec, light.base.transform.world);
					v = _uniforms_vec;
					break;
				}
			}
			case "_lightArea2": {
				if (light != null && light.data.size != null) {
					let f2: f32 = 0.5;
					let sx: f32 = light.data.size * f2;
					let sy: f32 = light.data.size_y * f2;
					vec4_set(_uniforms_vec, sx, -sy, 0.0);
					vec4_apply_mat(_uniforms_vec, light.base.transform.world);
					v = _uniforms_vec;
					break;
				}
			}
			case "_lightArea3": {
				if (light != null && light.data.size != null) {
					let f2: f32 = 0.5;
					let sx: f32 = light.data.size * f2;
					let sy: f32 = light.data.size_y * f2;
					vec4_set(_uniforms_vec, -sx, -sy, 0.0);
					vec4_apply_mat(_uniforms_vec, light.base.transform.world);
					v = _uniforms_vec;
					break;
				}
			}
			case "_cameraPosition": {
				vec4_set(_uniforms_vec, transform_world_x(camera.base.transform), transform_world_y(camera.base.transform), transform_world_z(camera.base.transform));
				v = _uniforms_vec;
				break;
			}
			case "_cameraLook": {
				_uniforms_vec = vec4_normalize(camera_object_look_world(camera));
				v = _uniforms_vec;
				break;
			}
			default:
				return false;
		}

		if (v != null) {
			g4_set_float3(location, v.x, v.y, v.z);
		}
		else {
			g4_set_float3(location, 0.0, 0.0, 0.0);
		}
		return true;
	}
	else if (c.type == "vec2") {
		let v: vec4_t = null;
		vec4_set(_uniforms_vec, 0, 0, 0);
		switch (c.link) {
			case "_vec2x": {
				v = _uniforms_vec;
				v.x = 1.0;
				v.y = 0.0;
				break;
			}
			case "_vec2xInv": {
				v = _uniforms_vec;
				v.x = 1.0 /render_path_current_w;
				v.y = 0.0;
				break;
			}
			case "_vec2x2": {
				v = _uniforms_vec;
				v.x = 2.0;
				v.y = 0.0;
				break;
			}
			case "_vec2x2Inv": {
				v = _uniforms_vec;
				v.x = 2.0 /render_path_current_w;
				v.y = 0.0;
				break;
			}
			case "_vec2y": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 1.0;
				break;
			}
			case "_vec2yInv": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 1.0 /render_path_current_h;
				break;
			}
			case "_vec2y2": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 2.0;
				break;
			}
			case "_vec2y2Inv": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 2.0 /render_path_current_h;
				break;
			}
			case "_vec2y3": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 3.0;
				break;
			}
			case "_vec2y3Inv": {
				v = _uniforms_vec;
				v.x = 0.0;
				v.y = 3.0 /render_path_current_h;
				break;
			}
			case "_screenSize": {
				v = _uniforms_vec;
				v.x = render_path_current_w;
				v.y = render_path_current_h;
				break;
			}
			case "_screenSizeInv": {
				v = _uniforms_vec;
				v.x = 1.0 /render_path_current_w;
				v.y = 1.0 /render_path_current_h;
				break;
			}
			case "_cameraPlaneProj": {
				let near = camera.data.near_plane;
				let far = camera.data.far_plane;
				v = _uniforms_vec;
				v.x = far / (far - near);
				v.y = (-far * near) / (far - near);
				break;
			}
			default:
				return false;
		}

		if (v != null) {
			g4_set_float2(location, v.x, v.y);
		}
		else {
			g4_set_float2(location, 0.0, 0.0);
		}
		return true;
	}
	else if (c.type == "float") {
		let f: Null<f32> = null;
		switch (c.link) {
			case "_time": {
				f = time_time();
				break;
			}
			case "_aspectRatioWindowF": {
				f = app_w() / app_h();
				break;
			}
			default:
				return false;
		}

		g4_set_float(location, f != null ? f : 0);
		return true;
	}
	else if (c.type == "floats") {
		let fa: Float32Array = null;
		switch (c.link) {
			case "_envmapIrradiance": {
				fa = scene_world == null ? world_data_get_empty_irradiance() : scene_world._irradiance;
				break;
			}
		}

		if (fa != null) {
			g4_set_floats(location, fa);
			return true;
		}
	}
	else if (c.type == "int") {
		let i: Null<i32> = null;
		switch (c.link) {
			case "_envmapNumMipmaps": {
				let w = scene_world;
				i = w != null ? w.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
				break;
			}
			default:
				return false;
		}

		g4_set_int(location, i != null ? i : 0);
		return true;
	}
	return false;
}

function uniforms_set_obj_const(g: g4_t, obj: object_t, loc: kinc_const_loc_t, c: shader_const_t) {
	if (c.link == null) return;

	let camera = scene_camera;
	let light =_render_path_light;

	if (c.type == "mat4") {
		let m: mat4_t = null;
		switch (c.link) {
			case "_worldMatrix": {
				m = obj.transform.world_unpack;
				break;
			}
			case "_inverseWorldMatrix": {
				mat4_get_inv(_uniforms_mat, obj.transform.world_unpack);
				m = _uniforms_mat;
				break;
			}
			case "_worldViewProjectionMatrix": {
				mat4_set_from(_uniforms_mat, obj.transform.world_unpack);
				mat4_mult_mat(_uniforms_mat, camera.v);
				mat4_mult_mat(_uniforms_mat, camera.p);
				m = _uniforms_mat;
				break;
			}
			case "_worldViewMatrix": {
				mat4_set_from(_uniforms_mat, obj.transform.world_unpack);
				mat4_mult_mat(_uniforms_mat, camera.v);
				m = _uniforms_mat;
				break;
			}
			case "_prevWorldViewProjectionMatrix": {
				mat4_set_from(_uniforms_mat, obj.ext.prev_matrix);
				mat4_mult_mat(_uniforms_mat, camera.prev_v);
				// helpMat.multmat(camera.prevP);
				mat4_mult_mat(_uniforms_mat, camera.p);
				m = _uniforms_mat;
				break;
			}
			///if arm_particles
			case "_particleData": {
				let mo = obj.ext;
				if (mo.particleOwner != null && mo.particleOwner.particleSystems != null) {
					m = particle_sys_get_data(mo.particleOwner.particleSystems[mo.particleIndex]);
				}
				break;
			}
			///end
		}

		if (m == null && uniforms_mat4_links != null) {
			for (let fn of uniforms_mat4_links) {
				m = fn(obj, current_material(obj), c.link);
				if (m != null) break;
			}
		}

		if (m == null) return;
		g4_set_mat(loc, m);
	}
	else if (c.type == "mat3") {
		let m: Mat3 = null;
		switch (c.link) {
			case "_normalMatrix": {
				mat4_get_inv(_uniforms_mat, obj.transform.world);
				mat4_transpose3x3(_uniforms_mat);
				mat3_setFrom4(_uniforms_mat3, _uniforms_mat);
				m = _uniforms_mat3;
				break;
			}
			case "_viewMatrix3": {
				mat3_setFrom4(_uniforms_mat3, camera.v);
				m = _uniforms_mat3;
				break;
			}
		}

		if (m == null) return;
		g4_set_mat3(loc, m);
	}
	else if (c.type == "vec4") {
		let v: vec4_t = null;
		vec4_set(_uniforms_vec, 0, 0, 0);

		if (v == null && uniforms_vec4_links != null) {
			for (let fn of uniforms_vec4_links) {
				v = fn(obj, current_material(obj), c.link);
				if (v != null) break;
			}
		}

		if (v == null) return;
		g4_set_float4(loc, v.x, v.y, v.z, v.w);
	}
	else if (c.type == "vec3") {
		let v: vec4_t = null;
		vec4_set(_uniforms_vec, 0, 0, 0);
		switch (c.link) {
			case "_dim": { // Model space
				let d = obj.transform.dim;
				let s = obj.transform.scale;
				vec4_set(_uniforms_vec, (d.x / s.x), (d.y / s.y), (d.z / s.z));
				v = _uniforms_vec;
				break;
			}
			case "_halfDim": { // Model space
				let d = obj.transform.dim;
				let s = obj.transform.scale;
				vec4_set(_uniforms_vec, (d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
				v = _uniforms_vec;
				break;
			}
		}

		if (v == null && uniforms_vec3_links != null) {
			for (let f of uniforms_vec3_links) {
				v = f(obj, current_material(obj), c.link);
				if (v != null) break;
			}
		}

		if (v == null) return;
		g4_set_float3(loc, v.x, v.y, v.z);
	}
	else if (c.type == "vec2") {
		let vx: Null<f32> = null;
		let vy: Null<f32> = null;

		if (vx == null && uniforms_vec2_links != null) {
			for (let fn of uniforms_vec2_links) {
				let v = fn(obj, current_material(obj), c.link);
				if (v != null) {
					vx = v.x;
					vy = v.y;
					break;
				}
			}
		}

		if (vx == null) return;
		g4_set_float2(loc, vx, vy);
	}
	else if (c.type == "float") {
		let f: Null<f32> = null;
		switch (c.link) {
			case "_objectInfoIndex": {
				f = obj.uid;
				break;
			}
			case "_objectInfoMaterialIndex": {
				f = current_material(obj)._uid;
				break;
			}
			case "_objectInfoRandom": {
				f = obj.urandom;
				break;
			}
			case "_posUnpack": {
				f = uniforms_pos_unpack != null ? uniforms_pos_unpack : 1.0;
				break;
			}
			case "_texUnpack": {
				f = uniforms_tex_unpack != null ? uniforms_tex_unpack : 1.0;
				break;
			}
		}

		if (f == null && uniforms_f32_links != null) {
			for (let fn of uniforms_f32_links) {
				let res = fn(obj, current_material(obj), c.link);
				if (res != null) {
					f = res;
					break;
				}
			}
		}

		if (f == null) return;
		g4_set_float(loc, f);
	}
	else if (c.type == "floats") {
		let fa: Float32Array = null;
		switch (c.link) {
			///if arm_skin
			case "_skinBones": {
				if (obj.animation != null) {
					fa = obj.animation.ext.skinBuffer;
				}
				break;
			}
			///end
		}

		if (fa == null && uniforms_f32_array_links != null) {
			for (let fn of uniforms_f32_array_links) {
				fa = fn(obj, current_material(obj), c.link);
				if (fa != null) break;
			}
		}

		if (fa == null) return;
		g4_set_floats(loc, fa);
	}
	else if (c.type == "int") {
		let i: Null<i32> = null;
		switch (c.link) {
			case "_uid": {
				i = obj.uid;
				break;
			}
		}

		if (i == null && uniforms_i32_links != null) {
			for (let fn of uniforms_i32_links) {
				let res = fn(obj, current_material(obj), c.link);
				if (res != null) {
					i = res;
					break;
				}
			}
		}

		if (i == null) return;
		g4_set_int(loc, i);
	}
}

function uniforms_set_material_consts(g: g4_t, context: shader_context_t, material_context: material_context_t) {
	if (material_context.bind_constants != null) {
		for (let i = 0; i < material_context.bind_constants.length; ++i) {
			let matc = material_context.bind_constants[i];
			let pos = -1;
			for (let i = 0; i < context.constants.length; ++i) {
				if (context.constants[i].name == matc.name) {
					pos = i;
					break;
				}
			}
			if (pos == -1) continue;
			let c = context.constants[pos];

			uniforms_set_material_const(g, context._constants[pos], c, matc);
		}
	}

	if (material_context._textures != null) {
		for (let i = 0; i < material_context._textures.length; ++i) {
			let mname = material_context.bind_textures[i].name;

			for (let j = 0; j < context._tex_units.length; ++j) {
				let sname = context.texture_units[j].name;
				if (mname == sname) {
					g4_set_tex(context._tex_units[j], material_context._textures[i]);
					// After texture sampler have been assigned, set texture parameters
					material_context_set_tex_params(material_context, g, i, context, j);
					break;
				}
			}
		}
	}
}

function current_material(object: object_t): material_data_t {
	if (object != null && object.ext != null && object.ext.materials != null) {
		let mo = object.ext;
		return mo.materials[mo.materialIndex];
	}
	return null;
}

function uniforms_set_material_const(g: g4_t, location: kinc_const_loc_t, shader_const: shader_const_t, material_const: bind_const_t) {
	switch (shader_const.type) {
		case "vec4":
			g4_set_float4(location, material_const.vec4[0], material_const.vec4[1], material_const.vec4[2], material_const.vec4[3]);
			break;
		case "vec3":
			g4_set_float3(location, material_const.vec3[0], material_const.vec3[1], material_const.vec3[2]);
			break;
		case "vec2":
			g4_set_float2(location, material_const.vec2[0], material_const.vec2[1]);
			break;
		case "float":
			g4_set_float(location,  material_const.float);
			break;
		case "bool":
			g4_set_bool(location, material_const.bool);
			break;
		case "int":
			g4_set_int(location, material_const.int);
			break;
	}
}

function uniforms_get_tex_addressing(s: string): TextureAddressing {
	switch (s) {
		case "clamp": return TextureAddressing.Clamp;
		case "mirror": return TextureAddressing.Mirror;
		default: return TextureAddressing.Repeat;
	}
}

function uniforms_get_tex_filter(s: string): TextureFilter {
	switch (s) {
		case "anisotropic": return TextureFilter.AnisotropicFilter;
		case "point": return TextureFilter.PointFilter;
		default: return TextureFilter.LinearFilter;
	}
}

function uniforms_get_mip_map_filter(s: string): MipMapFilter {
	switch (s) {
		case "linear": return MipMapFilter.LinearMipFilter;
		case "point": return MipMapFilter.PointMipFilter;
		default: return MipMapFilter.NoMipFilter;
	}
}
