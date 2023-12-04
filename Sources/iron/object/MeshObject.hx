package iron.object;

import haxe.ds.Vector;
import kha.Graphics4;
import kha.PipelineState;
import iron.math.Vec4;
import iron.math.Mat4;
import iron.data.MeshData;
import iron.data.MaterialData;
import iron.data.ShaderData;
import iron.data.SceneFormat;

class MeshObject extends Object {

	public var data: MeshData = null;
	public var materials: Vector<MaterialData>;
	public var materialIndex = 0;
	#if arm_particles
	public var particleSystems: Array<ParticleSystem> = null; // Particle owner
	public var particleChildren: Array<MeshObject> = null;
	public var particleOwner: MeshObject = null; // Particle object
	public var particleIndex = -1;
	#end
	public var cameraDistance: Float;
	public var screenSize = 0.0;
	public var frustumCulling = true;
	public var skip_context: String = null; // Do not draw this context
	public var force_context: String = null; // Draw only this context
	static var lastPipeline: PipelineState = null;
	public var prevMatrix = Mat4.identity();

	public function new(data: MeshData, materials: Vector<MaterialData>) {
		super();

		this.materials = materials;
		setData(data);
		Scene.active.meshes.push(this);
	}

	public function setData(data: MeshData) {
		this.data = data;
		data.refcount++;
		data.geom.build();

		// Scale-up packed (-1,1) mesh coords
		transform.scaleWorld = data.scalePos;
	}

	override public function remove() {
		#if arm_particles
		if (particleChildren != null) {
			for (c in particleChildren) c.remove();
			particleChildren = null;
		}
		if (particleSystems != null) {
			for (psys in particleSystems) psys.remove();
			particleSystems = null;
		}
		#end
		if (Scene.active != null) Scene.active.meshes.remove(this);
		data.refcount--;
		super.remove();
	}

	override public function setupAnimation(oactions: Array<TSceneFormat> = null) {
		#if arm_skin
		var hasAction = parent != null && parent.raw != null && parent.raw.bone_actions != null;
		if (hasAction) {
			var armatureName = parent.name;
			animation = getParentArmature(armatureName);
			if (animation == null) animation = new BoneAnimation(armatureName);
			if (data.isSkinned) cast(animation, BoneAnimation).setSkin(this);
		}
		#end
		super.setupAnimation(oactions);
	}

	#if arm_particles
	public function setupParticleSystem(sceneName: String, pref: TParticleReference) {
		if (particleSystems == null) particleSystems = [];
		var psys = new ParticleSystem(sceneName, pref);
		particleSystems.push(psys);
	}
	#end

	function setCulled(b: Bool): Bool {
		culled = b;
		return b;
	}

	public function cullMaterial(context: String): Bool {
		// Skip render if material does not contain current context
		var mats = materials;
		if (!validContext(mats, context)) return true;

		if (!visibleMesh) return setCulled(true);

		if (skip_context == context) return setCulled(true);
		if (force_context != null && force_context != context) return setCulled(true);

		return setCulled(false);
	}

	function cullMesh(context: String, camera: CameraObject, light: LightObject): Bool {
		if (camera == null) return false;

		if (camera.data.raw.frustum_culling && frustumCulling) {
			// Scale radius for skinned mesh and particle system
			// TODO: define skin & particle bounds
			var radiusScale = data.isSkinned ? 2.0 : 1.0;
			#if arm_particles
			// particleSystems for update, particleOwner for render
			if (particleSystems != null || particleOwner != null) radiusScale *= 1000;
			#end
			if (context == "voxel") radiusScale *= 100;
			if (data.geom.instanced) radiusScale *= 100;
			var frustumPlanes = camera.frustumPlanes;

			if (!CameraObject.sphereInFrustum(frustumPlanes, transform, radiusScale)) {
				return setCulled(true);
			}
		}

		culled = false;
		return culled;
	}

	function skipContext(context: String, mat: MaterialData): Bool {
		if (mat.raw.skip_context != null &&
			mat.raw.skip_context == context) {
			return true;
		}
		return false;
	}

	function getContexts(context: String, materials: Vector<MaterialData>, materialContexts: Array<MaterialContext>, shaderContexts: Array<ShaderContext>) {
		for (mat in materials) {
			var found = false;
			for (i in 0...mat.raw.contexts.length) {
				if (mat.raw.contexts[i].name.substr(0, context.length) == context) {
					materialContexts.push(mat.contexts[i]);
					shaderContexts.push(mat.shader.getContext(context));
					found = true;
					break;
				}
			}
			if (!found) {
				materialContexts.push(null);
				shaderContexts.push(null);
			}
		}
	}

	public function render(g: Graphics4, context: String, bindParams: Array<String>) {
		if (data == null || !data.geom.ready) return; // Data not yet streamed
		if (!visible) return; // Skip render if object is hidden
		if (cullMesh(context, Scene.active.camera, RenderPath.active.light)) return;
		var meshContext = raw != null ? context == "mesh" : false;

		#if arm_particles
		if (raw != null && raw.is_particle && particleOwner == null) return; // Instancing not yet set-up by particle system owner
		if (particleSystems != null && meshContext) {
			if (particleChildren == null) {
				particleChildren = [];
				for (psys in particleSystems) {
					// var c: MeshObject = cast Scene.active.getChild(psys.data.raw.instance_object);
					Scene.active.spawnObject(psys.data.raw.instance_object, null, function(o: Object) {
						if (o != null) {
							var c: MeshObject = cast o;
							particleChildren.push(c);
							c.particleOwner = this;
							c.particleIndex = particleChildren.length - 1;
						}
					});
				}
			}
			for (i in 0...particleSystems.length) {
				particleSystems[i].update(particleChildren[i], this);
			}
		}
		if (particleSystems != null && particleSystems.length > 0 && !raw.render_emitter) return;
		#end

		if (cullMaterial(context)) return;

		// Get context
		var materialContexts: Array<MaterialContext> = [];
		var shaderContexts: Array<ShaderContext> = [];
		getContexts(context, materials, materialContexts, shaderContexts);

		Uniforms.posUnpack = data.scalePos;
		Uniforms.texUnpack = data.scaleTex;
		transform.update();

		// Render mesh
		for (i in 0...data.geom.indexBuffers.length) {

			var mi = data.geom.materialIndices[i];
			if (shaderContexts.length <= mi || shaderContexts[mi] == null) continue;
			materialIndex = mi;

			// Check context skip
			if (materials.length > mi && skipContext(context, materials[mi])) continue;

			var scontext = shaderContexts[mi];
			if (scontext == null) continue;
			var elems = scontext.raw.vertex_elements;

			// Uniforms
			if (scontext.pipeState != lastPipeline) {
				g.setPipeline(scontext.pipeState);
				lastPipeline = scontext.pipeState;
				// Uniforms.setContextConstants(g, scontext, bindParams);
			}
			Uniforms.setContextConstants(g, scontext, bindParams); //
			Uniforms.setObjectConstants(g, scontext, this);
			if (materialContexts.length > mi) {
				Uniforms.setMaterialConstants(g, scontext, materialContexts[mi]);
			}

			// VB / IB
			if (data.geom.instancedVB != null) {
				g.setVertexBuffers([data.geom.get(elems), data.geom.instancedVB]);
			}
			else {
				g.setVertexBuffer(data.geom.get(elems));
			}

			g.setIndexBuffer(data.geom.indexBuffers[i]);

			// Draw
			if (data.geom.instanced) {
				g.drawIndexedVerticesInstanced(data.geom.instanceCount, data.geom.start, data.geom.count);
			}
			else {
				g.drawIndexedVertices(data.geom.start, data.geom.count);
			}
		}

		prevMatrix.setFrom(transform.worldUnpack);
	}

	function validContext(mats: Vector<MaterialData>, context: String): Bool {
		for (mat in mats) if (mat.getContext(context) != null) return true;
		return false;
	}

	public inline function computeCameraDistance(camX: Float, camY: Float, camZ: Float) {
		// Render path mesh sorting
		cameraDistance = Vec4.distancef(camX, camY, camZ, transform.worldx(), transform.worldy(), transform.worldz());
	}

	public inline function computeScreenSize(camera: CameraObject) {
		// Approx..
		// var rp = camera.renderPath;
		// var screenVolume = rp.currentW * rp.currentH;
		var tr = transform;
		var volume = tr.dim.x * tr.dim.y * tr.dim.z;
		screenSize = volume * (1.0 / cameraDistance);
		screenSize = screenSize > 1.0 ? 1.0 : screenSize;
	}
}
