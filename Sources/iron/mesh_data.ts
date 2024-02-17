
function mesh_data_parse(name: string, id: string): mesh_data_t {
	let format: scene_t = data_get_scene_raw(name);
	let raw: mesh_data_t = mesh_data_get_raw_by_name(format.mesh_datas, id);
	if (raw == null) {
		krom_log("Mesh data '" + id + "' not found!");
		return null;
	}

	let dat: mesh_data_t = mesh_data_create(raw);
	///if arm_skin
	if (raw.skin != null) {
		mesh_data_init_skeleton_transforms(dat, raw.skin.transforms_inv);
	}
	///end
	return dat;
}

function mesh_data_get_raw_by_name(datas: mesh_data_t[], name: string): mesh_data_t {
	if (name == "") {
		return datas[0];
	}
	for (let i: i32 = 0; i < datas.length; ++i) {
		if (datas[i].name == name) {
			return datas[i];
		}
	}
	return null;
}

function mesh_data_create(raw: mesh_data_t): mesh_data_t {
	if (raw.scale_pos == null) {
		raw.scale_pos = 1.0;
	}
	if (raw.scale_tex == null) {
		raw.scale_tex = 1.0;
	}

	raw._refcount = 0;
	raw._vertex_buffer_map = new Map();
	raw._ready = false;
	raw._instanced = false;
	raw._instance_count = 0;

	// Mesh data
	let indices: Uint32Array[] = [];
	let material_indices: i32[] = [];

	for (let i: i32 = 0; i < raw.index_arrays.length; ++i) {
		let ind: index_array_t = raw.index_arrays[i];
		indices.push(ind.values);
		material_indices.push(ind.material);
	}

	// Skinning
	// Prepare vertex array for skinning and fill size data
	let vertex_arrays: vertex_array_t[] = raw.vertex_arrays;
	if (raw.skin != null) {
		vertex_arrays.push({ attrib: "bone", values: null, data: "short4norm" });
		vertex_arrays.push({ attrib: "weight", values: null, data: "short4norm" });
	}
	for (let i: i32 = 0; i < vertex_arrays.length; ++i) {
		let padding: i32 = vertex_arrays[i].padding != null ? vertex_arrays[i].padding : 0;
		vertex_arrays[i]._size = mesh_data_get_vertex_size(vertex_arrays[i].data, padding);
	}

	if (raw.skin != null) {
		let vertex_length: i32 = Math.floor(vertex_arrays[0].values.length / vertex_arrays[0]._size);
		let l: i32 = vertex_length * 4;
		let bonea: Int16Array = new Int16Array(l);
		let weighta: Int16Array = new Int16Array(l);

		let index: i32 = 0;
		let ai: i32 = 0;
		for (let i: i32 = 0; i < vertex_length; ++i) {
			let bone_count: i32 = raw.skin.bone_count_array[i];
			for (let j: i32 = index; j < index + bone_count; ++j) {
				bonea[ai] = raw.skin.bone_index_array[j];
				weighta[ai] = raw.skin.bone_weight_array[j];
				ai++;
			}
			// Fill unused weights
			for (let j: i32 = bone_count; j < 4; ++j) {
				bonea[ai] = 0;
				weighta[ai] = 0;
				ai++;
			}
			index += bone_count;
		}
		vertex_arrays[vertex_arrays.length - 2].values = bonea;
		vertex_arrays[vertex_arrays.length - 1].values = weighta;
	}

	// Make vertex buffers
	raw._indices = indices;
	raw._material_indices = material_indices;
	raw._struct = mesh_data_get_vertex_struct(raw.vertex_arrays);

	return raw;
}

function mesh_data_get_vertex_struct(vertex_arrays: vertex_array_t[]): vertex_struct_t {
	let structure: vertex_struct_t = g4_vertex_struct_create();
	for (let i: i32 = 0; i < vertex_arrays.length; ++i) {
		g4_vertex_struct_add(structure, vertex_arrays[i].attrib, mesh_data_get_vertex_data(vertex_arrays[i].data));
	}
	return structure;
}

function mesh_data_get_vertex_data(data: string): vertex_data_t {
	if (data == "short4norm") {
		return vertex_data_t.I16_4X_NORM;
	}
	else if (data == "short2norm") {
		return vertex_data_t.I16_2X_NORM;
	}
	else {
		return vertex_data_t.I16_4X_NORM;
	}
}

function mesh_data_build_vertices(vertices: DataView, vertex_arrays: vertex_array_t[], offset: i32 = 0, fake_uvs: bool = false, uvs_index: i32 = -1) {
	let num_verts: i32 = vertex_arrays[0].values.length / vertex_arrays[0]._size;
	let di: i32 = -1 + offset;
	for (let i: i32 = 0; i < num_verts; ++i) {
		for (let va: i32 = 0; va < vertex_arrays.length; ++va) {
			let l: i32 = vertex_arrays[va]._size;
			if (fake_uvs && va == uvs_index) { // Add fake uvs if uvs where "asked" for but not found
				for (let j: i32 = 0; j < l; ++j) {
					vertices.setInt16(++di * 2, 0, true);
				}
				continue;
			}
			for (let o: i32 = 0; o < l; ++o) {
				vertices.setInt16(++di * 2, vertex_arrays[va].values[i * l + o], true);
			}
			if (vertex_arrays[va].padding != null) {
				if (vertex_arrays[va].padding == 1) {
					vertices.setInt16(++di * 2, 0, true);
				}
			}
		}
	}
}

function mesh_data_get_vertex_size(vertex_data: string, padding: i32 = 0): i32 {
	if (vertex_data == "short4norm") {
		return 4 - padding;
	}
	else if (vertex_data == "short2norm") {
		return 2 - padding;
	}
	else {
		return 0;
	}
}

function mesh_data_get_vertex_array(raw: mesh_data_t, name: string): vertex_array_t {
	for (let i = 0; i < raw.vertex_arrays.length; ++i) {
		if (raw.vertex_arrays[i].attrib == name) {
			return raw.vertex_arrays[i];
		}
	}
	return null;
}

function mesh_data_setup_inst(raw: mesh_data_t, data: Float32Array, inst_type: i32) {
	let structure: vertex_struct_t = g4_vertex_struct_create();
	structure.instanced = true;
	raw._instanced = true;
	// pos, pos+rot, pos+scale, pos+rot+scale
	g4_vertex_struct_add(structure, "ipos", vertex_data_t.F32_3X);
	if (inst_type == 2 || inst_type == 4) {
		g4_vertex_struct_add(structure, "irot", vertex_data_t.F32_3X);
	}
	if (inst_type == 3 || inst_type == 4) {
		g4_vertex_struct_add(structure, "iscl", vertex_data_t.F32_3X);
	}

	raw._instance_count = Math.floor(data.length / Math.floor(g4_vertex_struct_byte_size(structure) / 4));
	raw._instanced_vb = g4_vertex_buffer_create(raw._instance_count, structure, usage_t.STATIC, 1);
	let vertices: DataView = g4_vertex_buffer_lock(raw._instanced_vb);
	for (let i: i32 = 0; i < Math.floor(vertices.byteLength / 4); ++i) {
		vertices.setFloat32(i * 4, data[i], true);
	}
	g4_vertex_buffer_unlock(raw._instanced_vb);
}

function mesh_data_get(raw: mesh_data_t, vs: vertex_element_t[]): vertex_buffer_t {
	let key: string = "";
	for (let i: i32 = 0; i < vs.length; ++i) {
		let e: vertex_element_t = vs[i];
		key += e.name;
	}
	let vb: vertex_buffer_t = raw._vertex_buffer_map.get(key);
	if (vb == null) {
		let vertex_arrays: vertex_array_t[] = [];
		let has_tex: bool = false;
		let tex_offset: i32 = -1;
		let has_col: bool = false;
		for (let e = 0; e < vs.length; ++e) {
			if (vs[e].name == "tex") {
				has_tex = true;
				tex_offset = e;
			}
			if (vs[e].name == "col") {
				has_col = true;
			}
			for (let va = 0; va < raw.vertex_arrays.length; ++va) {
				if (vs[e].name == raw.vertex_arrays[va].attrib) {
					vertex_arrays.push(raw.vertex_arrays[va]);
				}
			}
		}
		// Multi-mat mesh with different vertex structures
		let positions: vertex_array_t = mesh_data_get_vertex_array(raw, 'pos');
		let uvs: vertex_array_t = mesh_data_get_vertex_array(raw, 'tex');
		let cols: vertex_array_t = mesh_data_get_vertex_array(raw, 'col');
		let struct: vertex_struct_t = mesh_data_get_vertex_struct(vertex_arrays);
		vb = g4_vertex_buffer_create(Math.floor(positions.values.length / positions._size), struct, usage_t.STATIC);
		raw._vertices = g4_vertex_buffer_lock(vb);
		mesh_data_build_vertices(raw._vertices, vertex_arrays, 0, has_tex && uvs == null, tex_offset);
		g4_vertex_buffer_unlock(vb);
		raw._vertex_buffer_map.set(key, vb);
		if (has_tex && uvs == null) {
			krom_log("Geometry " + raw.name + " is missing UV map");
		}
		if (has_col && cols == null) {
			krom_log("Geometry " + raw.name + " is missing vertex colors");
		}
	}
	return vb;
}

function mesh_data_build(raw: mesh_data_t) {
	if (raw._ready) {
		return;
	}

	let positions: vertex_array_t = mesh_data_get_vertex_array(raw, 'pos');
	raw._vertex_buffer = g4_vertex_buffer_create(Math.floor(positions.values.length / positions._size), raw._struct, usage_t.STATIC);
	raw._vertices = g4_vertex_buffer_lock(raw._vertex_buffer);
	mesh_data_build_vertices(raw._vertices, raw.vertex_arrays);
	g4_vertex_buffer_unlock(raw._vertex_buffer);

	let struct_str: string = "";
	for (let i: i32 = 0; i < raw._struct.elements.length; ++i) {
		let e: kinc_vertex_elem_t = raw._struct.elements[i];
		struct_str += e.name;
	}
	raw._vertex_buffer_map.set(struct_str, raw._vertex_buffer);

	raw._index_buffers = [];

	for (let i: i32 = 0; i < raw._indices.length; ++i) {
		let id: Uint32Array = raw._indices[i];
		if (id.length == 0) {
			continue;
		}
		let index_buffer: index_buffer_t = g4_index_buffer_create(id.length);

		let indices_array: Uint32Array = g4_index_buffer_lock(index_buffer);
		for (let i: i32 = 0; i < indices_array.length; ++i) {
			indices_array[i] = id[i];
		}

		g4_index_buffer_unlock(index_buffer);
		raw._index_buffers.push(index_buffer);
	}

	// Instanced
	if (raw.instanced_data != null) {
		mesh_data_setup_inst(raw, raw.instanced_data, raw.instanced_type);
	}

	raw._ready = true;
}

///if arm_skin
function mesh_data_add_armature(raw: mesh_data_t, armature: armature_t) {
	for (let i: i32 = 0; i < armature.actions.length; ++i) {
		let a: armature_action_t = armature.actions[i];
		mesh_data_add_action(raw, a.bones, a.name);
	}
}

function mesh_data_add_action(raw: mesh_data_t, bones: obj_t[], name: string) {
	if (bones == null) {
		return;
	}
	if (raw._actions == null) {
		raw._actions = new Map();
		raw._mats = new Map();
	}
	if (raw._actions.get(name) != null) {
		return;
	}
	let action_bones: obj_t[] = [];

	// Set bone references
	for (let i: i32 = 0; i < raw.skin.bone_ref_array.length; ++i) {
		let s: string = raw.skin.bone_ref_array[i];
		for (let j: i32 = 0; j < bones.length; ++j) {
			let b: obj_t = bones[j];
			if (b.name == s) {
				action_bones.push(b);
			}
		}
	}
	raw._actions.set(name, action_bones);

	let action_mats: mat4_t[] = [];
	for (let i: i32 = 0; i < action_bones.length; ++i) {
		let b: obj_t = action_bones[i];
		action_mats.push(mat4_from_f32_array(b.transform.values));
	}
	raw._mats.set(name, action_mats);
}

function mesh_data_init_skeleton_transforms(raw: mesh_data_t, transforms_inv: Float32Array[]) {
	raw._skeleton_transforms_inv = [];
	for (let i: i32 = 0; i < transforms_inv.length; ++i) {
		let t: Float32Array = transforms_inv[i];
		let mi = mat4_from_f32_array(t);
		raw._skeleton_transforms_inv.push(mi);
	}
}
///end

function mesh_data_calculate_aabb(raw: mesh_data_t): vec4_t {
	let aabb_min: vec4_t = vec4_create(-0.01, -0.01, -0.01);
	let aabb_max: vec4_t = vec4_create(0.01, 0.01, 0.01);
	let aabb: vec4_t = vec4_create();
	let i: i32 = 0;
	let positions: vertex_array_t = mesh_data_get_vertex_array(raw, 'pos');
	while (i < positions.values.length) {
		if (positions.values[i] > aabb_max.x) {
			aabb_max.x = positions.values[i];
		}
		if (positions.values[i + 1] > aabb_max.y) {
			aabb_max.y = positions.values[i + 1];
		}
		if (positions.values[i + 2] > aabb_max.z) {
			aabb_max.z = positions.values[i + 2];
		}
		if (positions.values[i] < aabb_min.x) {
			aabb_min.x = positions.values[i];
		}
		if (positions.values[i + 1] < aabb_min.y) {
			aabb_min.y = positions.values[i + 1];
		}
		if (positions.values[i + 2] < aabb_min.z) {
			aabb_min.z = positions.values[i + 2];
		}
		i += 4;
	}
	aabb.x = (Math.abs(aabb_min.x) + Math.abs(aabb_max.x)) / 32767 * raw.scale_pos;
	aabb.y = (Math.abs(aabb_min.y) + Math.abs(aabb_max.y)) / 32767 * raw.scale_pos;
	aabb.z = (Math.abs(aabb_min.z) + Math.abs(aabb_max.z)) / 32767 * raw.scale_pos;
	return aabb;
}

function mesh_data_delete(raw: mesh_data_t) {
	let buffer_map: vertex_buffer_t[] = Array.from(raw._vertex_buffer_map.values());
	for (let i: i32 = 0; i < buffer_map.length; ++i) {
		let buf = buffer_map[i];
		if (buf != null) {
			g4_vertex_buffer_delete(buf);
		}
	}
	for (let i: i32 = 0; i < raw._index_buffers.length; ++i) {
		let buf: index_buffer_t = raw._index_buffers[i];
		g4_index_buffer_delete(buf);
	}
}
