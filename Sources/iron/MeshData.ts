
class MeshData {

	static parse = (name: string, id: string, done: (md: TMeshData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TMeshData = Data.getMeshRawByName(format.mesh_datas, id);
			if (raw == null) {
				Krom.log(`Mesh data "${id}" not found!`);
				done(null);
			}

			MeshData.create(raw, (dat: TMeshData) => {
				///if arm_skin
				if (raw.skin != null) {
					MeshData.initSkeletonTransforms(dat, raw.skin.transformsI);
				}
				///end
				done(dat);
			});
		});
	}

	static create(raw: TMeshData, done: (md: TMeshData)=>void) {
		if (raw.scale_pos == null) raw.scale_pos = 1.0;
		if (raw.scale_tex == null) raw.scale_tex = 1.0;

		raw._refcount = 0;
		raw._vertexBufferMap = new Map();
		raw._ready = false;
		raw._instanced = false;
		raw._instanceCount = 0;

		// Mesh data
		let indices: Uint32Array[] = [];
		let materialIndices: i32[] = [];
		for (let ind of raw.index_arrays) {
			indices.push(ind.values);
			materialIndices.push(ind.material);
		}

		// Skinning
		// Prepare vertex array for skinning and fill size data
		let vertexArrays = raw.vertex_arrays;
		if (raw.skin != null) {
			vertexArrays.push({ attrib: "bone", values: null, data: "short4norm" });
			vertexArrays.push({ attrib: "weight", values: null, data: "short4norm" });
		}
		for (let i = 0; i < vertexArrays.length; ++i) {
			let padding = vertexArrays[i].padding != null ? vertexArrays[i].padding : 0;
			vertexArrays[i]._size = MeshData.getVertexSize(vertexArrays[i].data, padding);
		}

		if (raw.skin != null) {
			let bonea = null;
			let weighta = null;
			let vertex_length = Math.floor(vertexArrays[0].values.length / vertexArrays[0]._size);
			let l = vertex_length * 4;
			bonea = new Int16Array(l);
			weighta = new Int16Array(l);

			let index = 0;
			let ai = 0;
			for (let i = 0; i < vertex_length; ++i) {
				let boneCount = raw.skin.bone_count_array[i];
				for (let j = index; j < index + boneCount; ++j) {
					bonea[ai] = raw.skin.bone_index_array[j];
					weighta[ai] = raw.skin.bone_weight_array[j];
					ai++;
				}
				// Fill unused weights
				for (let j = boneCount; j < 4; ++j) {
					bonea[ai] = 0;
					weighta[ai] = 0;
					ai++;
				}
				index += boneCount;
			}
			vertexArrays[vertexArrays.length - 2].values = bonea;
			vertexArrays[vertexArrays.length - 1].values = weighta;
		}

		// Make vertex buffers
		raw._indices = indices;
		raw._materialIndices = materialIndices;
		raw._struct = MeshData.getVertexStructure(raw.vertex_arrays);

		done(raw);
	}

	static getVertexStructure = (vertexArrays: TVertexArray[]): VertexStructureRaw => {
		let structure = VertexStructure.create();
		for (let i = 0; i < vertexArrays.length; ++i) {
			VertexStructure.add(structure, vertexArrays[i].attrib, MeshData.getVertexData(vertexArrays[i].data));
		}
		return structure;
	}

	static getVertexData = (data: string): VertexData => {
		switch (data) {
			case "short4norm": return VertexData.I16_4X_Normalized;
			case "short2norm": return VertexData.I16_2X_Normalized;
			default: return VertexData.I16_4X_Normalized;
		}
	}

	static buildVertices = (vertices: DataView, vertexArrays: TVertexArray[], offset = 0, fakeUVs = false, uvsIndex = -1) => {
		let numVertices = vertexArrays[0].values.length / vertexArrays[0]._size;
		let di = -1 + offset;
		for (let i = 0; i < numVertices; ++i) {
			for (let va = 0; va < vertexArrays.length; ++va) {
				let l = vertexArrays[va]._size;
				if (fakeUVs && va == uvsIndex) { // Add fake uvs if uvs where "asked" for but not found
					for (let j = 0; j < l; ++j) vertices.setInt16(++di * 2, 0, true);
					continue;
				}
				for (let o  = 0; o < l; ++o) {
					vertices.setInt16(++di * 2, vertexArrays[va].values[i * l + o], true);
				}
				if (vertexArrays[va].padding != null) {
					if (vertexArrays[va].padding == 1) {
						vertices.setInt16(++di * 2, 0, true);
					}
				}
			}
		}
	}

	static getVertexSize = (vertex_data: string, padding: i32 = 0): i32 => {
		switch (vertex_data) {
			case "short4norm": return 4 - padding;
			case "short2norm": return 2 - padding;
			default: return 0;
		}
	}

	static getVArray = (raw: TMeshData, name: string): TVertexArray => {
		for (let i = 0; i < raw.vertex_arrays.length; ++i) {
			if (raw.vertex_arrays[i].attrib == name) {
				return raw.vertex_arrays[i];
			}
		}
		return null;
	}

	static setupInstanced = (raw: TMeshData, data: Float32Array, instancedType: i32) => {
		let structure = VertexStructure.create();
		structure.instanced = true;
		raw._instanced = true;
		// pos, pos+rot, pos+scale, pos+rot+scale
		VertexStructure.add(structure, "ipos", VertexData.F32_3X);
		if (instancedType == 2 || instancedType == 4) {
			VertexStructure.add(structure, "irot", VertexData.F32_3X);
		}
		if (instancedType == 3 || instancedType == 4) {
			VertexStructure.add(structure, "iscl", VertexData.F32_3X);
		}

		raw._instanceCount = Math.floor(data.length / Math.floor(VertexStructure.byteSize(structure) / 4));
		raw._instancedVB = VertexBuffer.create(raw._instanceCount, structure, Usage.StaticUsage, 1);
		let vertices = VertexBuffer.lock(raw._instancedVB);
		for (let i = 0; i < Math.floor(vertices.byteLength / 4); ++i) vertices.setFloat32(i * 4, data[i], true);
		VertexBuffer.unlock(raw._instancedVB);
	}

	static get = (raw: TMeshData, vs: TVertexElement[]): VertexBufferRaw => {
		let key = "";
		for (let e of vs) key += e.name;
		let vb = raw._vertexBufferMap.get(key);
		if (vb == null) {
			let vertexArrays = [];
			let hasTex = false;
			let texOffset = -1;
			let hasCol = false;
			for (let e = 0; e < vs.length; ++e) {
				if (vs[e].name == "tex") {
					hasTex = true;
					texOffset = e;
				}
				if (vs[e].name == "col") {
					hasCol = true;
				}
				for (let va = 0; va < raw.vertex_arrays.length; ++va) {
					if (vs[e].name == raw.vertex_arrays[va].attrib) {
						vertexArrays.push(raw.vertex_arrays[va]);
					}
				}
			}
			// Multi-mat mesh with different vertex structures
			let positions = MeshData.getVArray(raw, 'pos');
			let uvs = MeshData.getVArray(raw, 'tex');
			let cols = MeshData.getVArray(raw, 'col');
			let struct = MeshData.getVertexStructure(vertexArrays);
			vb = VertexBuffer.create(Math.floor(positions.values.length / positions._size), struct, Usage.StaticUsage);
			raw._vertices = VertexBuffer.lock(vb);
			MeshData.buildVertices(raw._vertices, vertexArrays, 0, hasTex && uvs == null, texOffset);
			VertexBuffer.unlock(vb);
			raw._vertexBufferMap.set(key, vb);
			if (hasTex && uvs == null) Krom.log("Armory Warning: Geometry " + raw.name + " is missing UV map");
			if (hasCol && cols == null) Krom.log("Armory Warning: Geometry " + raw.name + " is missing vertex colors");
		}
		return vb;
	}

	static build = (raw: TMeshData) => {
		if (raw._ready) return;

		let positions = MeshData.getVArray(raw, 'pos');
		raw._vertexBuffer = VertexBuffer.create(Math.floor(positions.values.length / positions._size), raw._struct, Usage.StaticUsage);
		raw._vertices = VertexBuffer.lock(raw._vertexBuffer);
		MeshData.buildVertices(raw._vertices, raw.vertex_arrays);
		VertexBuffer.unlock(raw._vertexBuffer);

		let structStr = "";
		for (let e of raw._struct.elements) structStr += e.name;
		raw._vertexBufferMap.set(structStr, raw._vertexBuffer);

		raw._indexBuffers = [];
		for (let id of raw._indices) {
			if (id.length == 0) continue;
			let indexBuffer = IndexBuffer.create(id.length, Usage.StaticUsage);

			let indicesA = IndexBuffer.lock(indexBuffer);
			for (let i = 0; i < indicesA.length; ++i) indicesA[i] = id[i];

			IndexBuffer.unlock(indexBuffer);
			raw._indexBuffers.push(indexBuffer);
		}

		// Instanced
		if (raw.instanced_data != null) MeshData.setupInstanced(raw, raw.instanced_data, raw.instanced_type);

		raw._ready = true;
	}

	///if arm_skin
	static addArmature = (raw: TMeshData, armature: TArmature) => {
		for (let a of armature.actions) {
			MeshData.addAction(raw, a.bones, a.name);
		}
	}

	static addAction = (raw: TMeshData, bones: TObj[], name: string) => {
		if (bones == null) return;
		if (raw._actions == null) {
			raw._actions = new Map();
			raw._mats = new Map();
		}
		if (raw._actions.get(name) != null) return;
		let actionBones: TObj[] = [];

		// Set bone references
		for (let s of raw.skin.bone_ref_array) {
			for (let b of bones) {
				if (b.name == s) {
					actionBones.push(b);
				}
			}
		}
		raw._actions.set(name, actionBones);

		let actionMats: TMat4[] = [];
		for (let b of actionBones) {
			actionMats.push(Mat4.fromFloat32Array(b.transform.values));
		}
		raw._mats.set(name, actionMats);
	}

	static initSkeletonTransforms = (raw: TMeshData, transformsI: Float32Array[]) => {
		raw._skeletonTransformsI = [];
		for (let t of transformsI) {
			let mi = Mat4.fromFloat32Array(t);
			raw._skeletonTransformsI.push(mi);
		}
	}
	///end

	static calculateAABB = (raw: TMeshData): TVec4 => {
		let aabbMin = Vec4.create(-0.01, -0.01, -0.01);
		let aabbMax = Vec4.create(0.01, 0.01, 0.01);
		let aabb = Vec4.create();
		let i = 0;
		let positions = MeshData.getVArray(raw, 'pos');
		while (i < positions.values.length) {
			if (positions.values[i    ] > aabbMax.x) aabbMax.x = positions.values[i];
			if (positions.values[i + 1] > aabbMax.y) aabbMax.y = positions.values[i + 1];
			if (positions.values[i + 2] > aabbMax.z) aabbMax.z = positions.values[i + 2];
			if (positions.values[i    ] < aabbMin.x) aabbMin.x = positions.values[i];
			if (positions.values[i + 1] < aabbMin.y) aabbMin.y = positions.values[i + 1];
			if (positions.values[i + 2] < aabbMin.z) aabbMin.z = positions.values[i + 2];
			i += 4;
		}
		aabb.x = (Math.abs(aabbMin.x) + Math.abs(aabbMax.x)) / 32767 * raw.scale_pos;
		aabb.y = (Math.abs(aabbMin.y) + Math.abs(aabbMax.y)) / 32767 * raw.scale_pos;
		aabb.z = (Math.abs(aabbMin.z) + Math.abs(aabbMax.z)) / 32767 * raw.scale_pos;
		return aabb;
	}

	static delete = (raw: TMeshData) => {
		for (let buf of raw._vertexBufferMap.values()) if (buf != null) VertexBuffer.delete(buf);
		for (let buf of raw._indexBuffers) IndexBuffer.delete(buf);
	}
}

