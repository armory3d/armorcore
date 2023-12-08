package iron.data;

import js.lib.DataView;
import js.lib.Float32Array;
import js.lib.Uint32Array;
import js.lib.Int16Array;
import iron.System;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.data.SceneFormat;
import iron.data.MeshData;

class Geometry {
	public var vertexBuffer: VertexBuffer;
	public var vertexBufferMap: Map<String, VertexBuffer> = new Map();

	public var indexBuffers: Array<IndexBuffer>;
	public var start = 0; // For drawIndexedVertices
	public var count = -1;
	public var name = "";

	public var ready = false;
	public var vertices: DataView;
	public var indices: Array<Uint32Array>;
	public var numTris = 0;
	public var materialIndices: Array<Int>;
	public var struct: VertexStructure;
	public var structLength: Int;
	public var structStr: String;
	public var usage: Usage;

	public var instancedVB: VertexBuffer = null;
	public var instanced = false;
	public var instanceCount = 0;

	public var positions: TVertexArray;
	public var normals: TVertexArray;
	public var uvs: TVertexArray;
	public var cols: TVertexArray;
	public var vertexArrays: Array<TVertexArray>;
	var data: MeshData;

	public var aabb: Vec4 = null;
	public var aabbMin: Vec4 = null;
	public var aabbMax: Vec4 = null;

#if arm_skin
	public var skinBoneCounts: Int16Array = null;
	public var skinBoneIndices: Int16Array = null;
	public var skinBoneWeights: Int16Array = null;

	public var skeletonTransformsI: Array<Mat4> = null;
	public var skeletonBoneRefs: Array<String> = null;
	public var skeletonBoneLens: Float32Array = null;

	public var actions: Map<String, Array<TObj>> = null;
	public var mats: Map<String, Array<Mat4>> = null;
#end

	public function new(data: MeshData, indices: Array<Uint32Array>, materialIndices: Array<Int>) {
		this.indices = indices;
		this.materialIndices = materialIndices;
		this.usage = Usage.StaticUsage;

		this.vertexArrays = data.raw.vertex_arrays;
		this.positions = getVArray('pos');
		this.normals = getVArray('nor');
		this.uvs = getVArray('tex');
		this.cols = getVArray('col');
		this.data = data;

		struct = getVertexStructure(vertexArrays);
		structLength = Std.int(struct.byteSize() / 2);
		structStr = "";
		for (e in struct.elements) structStr += e.name;
	}

	public function delete() {
		for (buf in vertexBufferMap) if (buf != null) buf.delete();
		for (buf in indexBuffers) buf.delete();
	}

	static function getVertexStructure(vertexArrays: Array<TVertexArray>): VertexStructure {
		var structure = new VertexStructure();
		for (i in 0...vertexArrays.length) {
			structure.add(vertexArrays[i].attrib, getVertexData(vertexArrays[i].data));
		}
		return structure;
	}

	static function getVertexData(data: String): VertexData {
		switch (data) {
			case "short4norm": return VertexData.I16_4X_Normalized;
			case "short2norm": return VertexData.I16_2X_Normalized;
			default: return VertexData.I16_4X_Normalized;
		}
	}

	public function getVArray(name: String): TVertexArray {
		for (i in 0...vertexArrays.length) {
			if (vertexArrays[i].attrib == name) {
				return vertexArrays[i];
			}
		}
		return null;
	}

	public function setupInstanced(data: Float32Array, instancedType: Int, usage: Usage) {
		var structure = new VertexStructure();
		structure.instanced = true;
		instanced = true;
		// pos, pos+rot, pos+scale, pos+rot+scale
		structure.add("ipos", VertexData.F32_3X);
		if (instancedType == 2 || instancedType == 4) {
			structure.add("irot", VertexData.F32_3X);
		}
		if (instancedType == 3 || instancedType == 4) {
			structure.add("iscl", VertexData.F32_3X);
		}

		instanceCount = Std.int(data.length / Std.int(structure.byteSize() / 4));
		instancedVB = new VertexBuffer(instanceCount, structure, usage, 1);
		var vertices = instancedVB.lock();
		for (i in 0...Std.int(vertices.byteLength / 4)) vertices.setFloat32(i * 4, data[i], true);
		instancedVB.unlock();
	}

	static function buildVertices(vertices: DataView, vertexArrays: Array<TVertexArray>, offset = 0, fakeUVs = false, uvsIndex = -1) {
		var numVertices = verticesCount(vertexArrays[0]);
		var di = -1 + offset;
		for (i in 0...numVertices) {
			for (va in 0...vertexArrays.length) {
				var l = vertexArrays[va].size;
				if (fakeUVs && va == uvsIndex) { // Add fake uvs if uvs where "asked" for but not found
					for (j in 0...l) vertices.setInt16(++di * 2, 0, true);
					continue;
				}
				for (o in 0...l) {
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

	public function get(vs: Array<TVertexElement>): VertexBuffer {
		var key = "";
		for (e in vs) key += e.name;
		var vb = vertexBufferMap.get(key);
		if (vb == null) {
			var nVertexArrays = [];
			var atex = false;
			var texOffset = -1;
			var acol = false;
			for (e in 0...vs.length) {
				if (vs[e].name == "tex") {
					atex = true;
					texOffset = e;
				}
				if (vs[e].name == "col") {
					acol = true;
				}
				for (va in 0...vertexArrays.length) {
					if (vs[e].name == vertexArrays[va].attrib) {
						nVertexArrays.push(vertexArrays[va]);
					}
				}
			}
			// Multi-mat mesh with different vertex structures
			var struct = getVertexStructure(nVertexArrays);
			vb = new VertexBuffer(Std.int(positions.values.length / positions.size), struct, usage);
			vertices = vb.lock();
			buildVertices(vertices, nVertexArrays, 0, atex && uvs == null, texOffset);
			vb.unlock();
			vertexBufferMap.set(key, vb);
			if (atex && uvs == null) trace("Armory Warning: Geometry " + name + " is missing UV map");
			if (acol && cols == null) trace("Armory Warning: Geometry " + name + " is missing vertex colors");
		}
		return vb;
	}

	public function build() {
		if (ready) return;

		vertexBuffer = new VertexBuffer(Std.int(positions.values.length / positions.size), struct, usage);
		vertices = vertexBuffer.lock();
		buildVertices(vertices, vertexArrays);
		vertexBuffer.unlock();
		vertexBufferMap.set(structStr, vertexBuffer);

		indexBuffers = [];
		for (id in indices) {
			if (id.length == 0) continue;
			var indexBuffer = new IndexBuffer(id.length, usage);
			numTris += Std.int(id.length / 3);

			var indicesA = indexBuffer.lock();
			for (i in 0...indicesA.length) indicesA[i] = id[i];

			indexBuffer.unlock();
			indexBuffers.push(indexBuffer);
		}

		// Instanced
		if (data.raw.instanced_data != null) setupInstanced(data.raw.instanced_data, data.raw.instanced_type, usage);

		ready = true;
	}

	inline static function verticesCount(arr: TVertexArray): Int {
		return Std.int(arr.values.length / arr.size);
	}

#if arm_skin
	public function addArmature(armature: Armature) {
		for (a in armature.actions) {
			addAction(a.bones, a.name);
		}
	}

	public function addAction(bones: Array<TObj>, name: String) {
		if (bones == null) return;
		if (actions == null) {
			actions = new Map();
			mats = new Map();
		}
		if (actions.get(name) != null) return;
		var actionBones: Array<TObj> = [];

		// Set bone references
		for (s in skeletonBoneRefs) {
			for (b in bones) {
				if (b.name == s) {
					actionBones.push(b);
				}
			}
		}
		actions.set(name, actionBones);

		var actionMats: Array<Mat4> = [];
		for (b in actionBones) {
			actionMats.push(Mat4.fromFloat32Array(b.transform.values));
		}
		mats.set(name, actionMats);
	}

	public function initSkeletonTransforms(transformsI: Array<Float32Array>) {
		skeletonTransformsI = [];
		for (t in transformsI) {
			var mi = Mat4.fromFloat32Array(t);
			skeletonTransformsI.push(mi);
		}
	}
#end

	public function calculateAABB() {
		aabbMin = new Vec4(-0.01, -0.01, -0.01);
		aabbMax = new Vec4(0.01, 0.01, 0.01);
		aabb = new Vec4();
		var i = 0;
		while (i < positions.values.length) {
			if (positions.values[i    ] > aabbMax.x) aabbMax.x = positions.values[i];
			if (positions.values[i + 1] > aabbMax.y) aabbMax.y = positions.values[i + 1];
			if (positions.values[i + 2] > aabbMax.z) aabbMax.z = positions.values[i + 2];
			if (positions.values[i    ] < aabbMin.x) aabbMin.x = positions.values[i];
			if (positions.values[i + 1] < aabbMin.y) aabbMin.y = positions.values[i + 1];
			if (positions.values[i + 2] < aabbMin.z) aabbMin.z = positions.values[i + 2];
			i += 4;
		}
		aabb.x = (Math.abs(aabbMin.x) + Math.abs(aabbMax.x)) / 32767 * data.scalePos;
		aabb.y = (Math.abs(aabbMin.y) + Math.abs(aabbMax.y)) / 32767 * data.scalePos;
		aabb.z = (Math.abs(aabbMin.z) + Math.abs(aabbMax.z)) / 32767 * data.scalePos;
	}
}
