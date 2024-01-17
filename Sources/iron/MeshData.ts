
class MeshData {

	name: string;
	raw: TMeshData;
	format: TSceneFormat;
	refcount = 0; // Number of users
	handle: string; // Handle used to retrieve this object in Data
	scalePos: f32 = 1.0;
	scaleTex: f32 = 1.0;
	isSkinned: bool;

	vertexBuffer: VertexBuffer;
	vertexBufferMap: Map<string, VertexBuffer> = new Map();

	indexBuffers: IndexBuffer[];

	ready = false;
	vertices: DataView;
	indices: Uint32Array[];
	numTris = 0;
	materialIndices: i32[];
	struct: VertexStructure;
	structLength: i32;
	structStr: string;
	usage: Usage;

	instancedVB: VertexBuffer = null;
	instanced = false;
	instanceCount = 0;

	positions: TVertexArray;
	normals: TVertexArray;
	uvs: TVertexArray;
	cols: TVertexArray;
	vertexArrays: TVertexArray[];

	aabb: Vec4 = null;
	aabbMin: Vec4 = null;
	aabbMax: Vec4 = null;

	///if arm_skin
	skinBoneCounts: Int16Array = null;
	skinBoneIndices: Int16Array = null;
	skinBoneWeights: Int16Array = null;

	skeletonTransformsI: Mat4[] = null;
	skeletonBoneRefs: string[] = null;
	skeletonBoneLens: Float32Array = null;

	actions: Map<string, TObj[]> = null;
	mats: Map<string, Mat4[]> = null;
	///end

	static getVertexStructure = (vertexArrays: TVertexArray[]): VertexStructure => {
		let structure = new VertexStructure();
		for (let i = 0; i < vertexArrays.length; ++i) {
			structure.add(vertexArrays[i].attrib, MeshData.getVertexData(vertexArrays[i].data));
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

	getVArray = (name: string): TVertexArray => {
		for (let i = 0; i < this.vertexArrays.length; ++i) {
			if (this.vertexArrays[i].attrib == name) {
				return this.vertexArrays[i];
			}
		}
		return null;
	}

	setupInstanced = (data: Float32Array, instancedType: i32, usage: Usage) => {
		let structure = new VertexStructure();
		structure.instanced = true;
		this.instanced = true;
		// pos, pos+rot, pos+scale, pos+rot+scale
		structure.add("ipos", VertexData.F32_3X);
		if (instancedType == 2 || instancedType == 4) {
			structure.add("irot", VertexData.F32_3X);
		}
		if (instancedType == 3 || instancedType == 4) {
			structure.add("iscl", VertexData.F32_3X);
		}

		this.instanceCount = Math.floor(data.length / Math.floor(structure.byteSize() / 4));
		this.instancedVB = new VertexBuffer(this.instanceCount, structure, usage, 1);
		let vertices = this.instancedVB.lock();
		for (let i = 0; i < Math.floor(vertices.byteLength / 4); ++i) vertices.setFloat32(i * 4, data[i], true);
		this.instancedVB.unlock();
	}

	static buildVertices = (vertices: DataView, vertexArrays: TVertexArray[], offset = 0, fakeUVs = false, uvsIndex = -1) => {
		let numVertices = MeshData.verticesCount(vertexArrays[0]);
		let di = -1 + offset;
		for (let i = 0; i < numVertices; ++i) {
			for (let va = 0; va < vertexArrays.length; ++va) {
				let l = vertexArrays[va].size;
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

	get = (vs: TVertexElement[]): VertexBuffer => {
		let key = "";
		for (let e of vs) key += e.name;
		let vb = this.vertexBufferMap.get(key);
		if (vb == null) {
			let nVertexArrays = [];
			let atex = false;
			let texOffset = -1;
			let acol = false;
			for (let e = 0; e < vs.length; ++e) {
				if (vs[e].name == "tex") {
					atex = true;
					texOffset = e;
				}
				if (vs[e].name == "col") {
					acol = true;
				}
				for (let va = 0; va < this.vertexArrays.length; ++va) {
					if (vs[e].name == this.vertexArrays[va].attrib) {
						nVertexArrays.push(this.vertexArrays[va]);
					}
				}
			}
			// Multi-mat mesh with different vertex structures
			let struct = MeshData.getVertexStructure(nVertexArrays);
			vb = new VertexBuffer(Math.floor(this.positions.values.length / this.positions.size), struct, this.usage);
			this.vertices = vb.lock();
			MeshData.buildVertices(this.vertices, nVertexArrays, 0, atex && this.uvs == null, texOffset);
			vb.unlock();
			this.vertexBufferMap.set(key, vb);
			if (atex && this.uvs == null) Krom.log("Armory Warning: Geometry " + this.name + " is missing UV map");
			if (acol && this.cols == null) Krom.log("Armory Warning: Geometry " + this.name + " is missing vertex colors");
		}
		return vb;
	}

	build = () => {
		if (this.ready) return;

		this.vertexBuffer = new VertexBuffer(Math.floor(this.positions.values.length / this.positions.size), this.struct, this.usage);
		this.vertices = this.vertexBuffer.lock();
		MeshData.buildVertices(this.vertices, this.vertexArrays);
		this.vertexBuffer.unlock();
		this.vertexBufferMap.set(this.structStr, this.vertexBuffer);

		this.indexBuffers = [];
		for (let id of this.indices) {
			if (id.length == 0) continue;
			let indexBuffer = new IndexBuffer(id.length, this.usage);
			this.numTris += Math.floor(id.length / 3);

			let indicesA = indexBuffer.lock();
			for (let i = 0; i < indicesA.length; ++i) indicesA[i] = id[i];

			indexBuffer.unlock();
			this.indexBuffers.push(indexBuffer);
		}

		// Instanced
		if (this.raw.instanced_data != null) this.setupInstanced(this.raw.instanced_data, this.raw.instanced_type, this.usage);

		this.ready = true;
	}

	static verticesCount = (arr: TVertexArray): i32 => {
		return Math.floor(arr.values.length / arr.size);
	}

	///if arm_skin
	addArmature = (armature: Armature) => {
		for (let a of armature.actions) {
			this.addAction(a.bones, a.name);
		}
	}

	addAction = (bones: TObj[], name: string) => {
		if (bones == null) return;
		if (this.actions == null) {
			this.actions = new Map();
			this.mats = new Map();
		}
		if (this.actions.get(name) != null) return;
		let actionBones: TObj[] = [];

		// Set bone references
		for (let s of this.skeletonBoneRefs) {
			for (let b of bones) {
				if (b.name == s) {
					actionBones.push(b);
				}
			}
		}
		this.actions.set(name, actionBones);

		let actionMats: Mat4[] = [];
		for (let b of actionBones) {
			actionMats.push(Mat4.fromFloat32Array(b.transform.values));
		}
		this.mats.set(name, actionMats);
	}

	initSkeletonTransforms = (transformsI: Float32Array[]) => {
		this.skeletonTransformsI = [];
		for (let t of transformsI) {
			let mi = Mat4.fromFloat32Array(t);
			this.skeletonTransformsI.push(mi);
		}
	}
	///end

	calculateAABB = () => {
		this.aabbMin = new Vec4(-0.01, -0.01, -0.01);
		this.aabbMax = new Vec4(0.01, 0.01, 0.01);
		this.aabb = new Vec4();
		let i = 0;
		while (i < this.positions.values.length) {
			if (this.positions.values[i    ] > this.aabbMax.x) this.aabbMax.x = this.positions.values[i];
			if (this.positions.values[i + 1] > this.aabbMax.y) this.aabbMax.y = this.positions.values[i + 1];
			if (this.positions.values[i + 2] > this.aabbMax.z) this.aabbMax.z = this.positions.values[i + 2];
			if (this.positions.values[i    ] < this.aabbMin.x) this.aabbMin.x = this.positions.values[i];
			if (this.positions.values[i + 1] < this.aabbMin.y) this.aabbMin.y = this.positions.values[i + 1];
			if (this.positions.values[i + 2] < this.aabbMin.z) this.aabbMin.z = this.positions.values[i + 2];
			i += 4;
		}
		this.aabb.x = (Math.abs(this.aabbMin.x) + Math.abs(this.aabbMax.x)) / 32767 * this.scalePos;
		this.aabb.y = (Math.abs(this.aabbMin.y) + Math.abs(this.aabbMax.y)) / 32767 * this.scalePos;
		this.aabb.z = (Math.abs(this.aabbMin.z) + Math.abs(this.aabbMax.z)) / 32767 * this.scalePos;
	}

	constructor(raw: TMeshData, done: (md: MeshData)=>void) {
		this.raw = raw;
		this.name = raw.name;

		if (raw.scale_pos != null) this.scalePos = raw.scale_pos;
		if (raw.scale_tex != null) this.scaleTex = raw.scale_tex;

		// Mesh data
		let indices: Uint32Array[] = [];
		let materialIndices: i32[] = [];
		for (let ind of raw.index_arrays) {
			indices.push(ind.values);
			materialIndices.push(ind.material);
		}

		// Skinning
		this.isSkinned = raw.skin != null;

		// Prepare vertex array for skinning and fill size data
		let vertexArrays = raw.vertex_arrays;
		if (this.isSkinned) {
			vertexArrays.push({ attrib: "bone", values: null, data: "short4norm" });
			vertexArrays.push({ attrib: "weight", values: null, data: "short4norm" });
		}
		for (let i = 0; i < vertexArrays.length; ++i) {
			vertexArrays[i].size = this.getVertexSize(vertexArrays[i].data, this.getPadding(vertexArrays[i].padding));
		}

		if (this.isSkinned) {
			let bonea = null;
			let weighta = null;
			let vertex_length = Math.floor(vertexArrays[0].values.length / vertexArrays[0].size);
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
		this.indices = indices;
		this.materialIndices = materialIndices;
		this.usage = Usage.StaticUsage;

		this.vertexArrays = raw.vertex_arrays;
		this.positions = this.getVArray('pos');
		this.normals = this.getVArray('nor');
		this.uvs = this.getVArray('tex');
		this.cols = this.getVArray('col');

		this.struct = MeshData.getVertexStructure(vertexArrays);
		this.structLength = Math.floor(this.struct.byteSize() / 2);
		this.structStr = "";
		for (let e of this.struct.elements) this.structStr += e.name;

		done(this);
	}

	delete = () => {
		for (let buf of this.vertexBufferMap.values()) if (buf != null) buf.delete();
		for (let buf of this.indexBuffers) buf.delete();
	}

	static parse = (name: string, id: string, done: (md: MeshData)=>void) => {
		Data.getSceneRaw(name, (format: TSceneFormat) => {
			let raw: TMeshData = Data.getMeshRawByName(format.mesh_datas, id);
			if (raw == null) {
				Krom.log(`Mesh data "${id}" not found!`);
				done(null);
			}

			new MeshData(raw, (dat: MeshData) => {
				dat.format = format;
				///if arm_skin
				if (raw.skin != null) {
					dat.skinBoneCounts = raw.skin.bone_count_array;
					dat.skinBoneIndices = raw.skin.bone_index_array;
					dat.skinBoneWeights = raw.skin.bone_weight_array;
					dat.skeletonBoneRefs = raw.skin.bone_ref_array;
					dat.skeletonBoneLens = raw.skin.bone_len_array;
					dat.initSkeletonTransforms(raw.skin.transformsI);
				}
				///end
				done(dat);
			});
		});
	}

	getVertexSize = (vertex_data: string, padding: i32 = 0): i32 => {
		switch (vertex_data) {
			case "short4norm": return 4 - padding;
			case "short2norm": return 2 - padding;
			default: return 0;
		}
	}

	getPadding = (padding: Null<i32>): i32 => {
		return padding != null ? padding : 0;
	}
}

