
class MeshObject {

	base: BaseObject;
	data: TMeshData = null;
	materials: TMaterialData[];
	materialIndex = 0;
	///if arm_particles
	particleSystems: TParticleSystem[] = null; // Particle owner
	particleChildren: MeshObject[] = null;
	particleOwner: MeshObject = null; // Particle object
	particleIndex = -1;
	///end
	cameraDistance: f32;
	screenSize = 0.0;
	frustumCulling = true;
	skip_context: string = null; // Do not draw this context
	force_context: string = null; // Draw only this context
	prevMatrix = Mat4.identity();

	static lastPipeline: PipelineState = null;

	constructor(data: TMeshData, materials: TMaterialData[]) {
		this.base = new BaseObject();
		this.base.ext = this;
		this.base.remove = this.remove;
		this.base.setupAnimation = this.setupAnimation;

		this.materials = materials;
		this.setData(data);
		Scene.meshes.push(this);
	}

	setData = (data: TMeshData) => {
		this.data = data;
		data._refcount++;
		MeshData.build(data);

		// Scale-up packed (-1,1) mesh coords
		this.base.transform.scaleWorld = data.scale_pos;
	}

	remove = () => {
		///if arm_particles
		if (this.particleChildren != null) {
			for (let c of this.particleChildren) c.remove();
			this.particleChildren = null;
		}
		if (this.particleSystems != null) {
			for (let psys of this.particleSystems) ParticleSystem.remove(psys);
			this.particleSystems = null;
		}
		///end
		array_remove(Scene.meshes, this);
		this.data._refcount--;
		this.base.removeSuper();
	}

	setupAnimation = (oactions: TSceneFormat[] = null) => {
		///if arm_skin
		let hasAction = this.base.parent != null && this.base.parent.raw != null && this.base.parent.raw.bone_actions != null;
		if (hasAction) {
			let armatureName = this.base.parent.name;
			this.base.animation = this.base.getParentArmature(armatureName).base;
			if (this.base.animation == null) this.base.animation = BoneAnimation.create(armatureName).base;
			if (this.data.skin != null) BoneAnimation.setSkin(this.base.animation.ext, this);
		}
		///end
		this.base.setupAnimationSuper(oactions);
	}

	///if arm_particles
	setupParticleSystem = (sceneName: string, pref: TParticleReference) => {
		if (this.particleSystems == null) this.particleSystems = [];
		let psys = ParticleSystem.create(sceneName, pref);
		this.particleSystems.push(psys);
	}
	///end

	setCulled = (b: bool): bool => {
		this.base.culled = b;
		return b;
	}

	cullMaterial = (context: string): bool => {
		// Skip render if material does not contain current context
		let mats = this.materials;
		if (!this.validContext(mats, context)) return true;

		if (!this.base.visible) return this.setCulled(true);

		if (this.skip_context == context) return this.setCulled(true);
		if (this.force_context != null && this.force_context != context) return this.setCulled(true);

		return this.setCulled(false);
	}

	cullMesh = (context: string, camera: CameraObject, light: LightObject): bool => {
		if (camera == null) return false;

		if (camera.data.frustum_culling && this.frustumCulling) {
			// Scale radius for skinned mesh and particle system
			// TODO: define skin & particle bounds
			let radiusScale = this.data.skin != null ? 2.0 : 1.0;
			///if arm_particles
			// particleSystems for update, particleOwner for render
			if (this.particleSystems != null || this.particleOwner != null) radiusScale *= 1000;
			///end
			if (context == "voxel") radiusScale *= 100;
			if (this.data._instanced) radiusScale *= 100;
			let frustumPlanes = camera.frustumPlanes;

			if (!CameraObject.sphereInFrustum(frustumPlanes, this.base.transform, radiusScale)) {
				return this.setCulled(true);
			}
		}

		this.base.culled = false;
		return this.base.culled;
	}

	skipContext = (context: string, mat: TMaterialData): bool => {
		if (mat.skip_context != null &&
			mat.skip_context == context) {
			return true;
		}
		return false;
	}

	getContexts = (context: string, materials: TMaterialData[], materialContexts: TMaterialContext[], shaderContexts: TShaderContext[]) => {
		for (let mat of materials) {
			let found = false;
			for (let i = 0; i < mat.contexts.length; ++i) {
				if (mat.contexts[i].name.substr(0, context.length) == context) {
					materialContexts.push(mat._contexts[i]);
					shaderContexts.push(ShaderData.getContext(mat._shader, context));
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

	render = (g: Graphics4, context: string, bindParams: string[]) => {
		if (this.data == null || !this.data._ready) return; // Data not yet streamed
		if (!this.base.visible) return; // Skip render if object is hidden
		if (this.cullMesh(context, Scene.camera, RenderPath.light)) return;
		let meshContext = this.base.raw != null ? context == "mesh" : false;

		///if arm_particles
		if (this.base.raw != null && this.base.raw.is_particle && this.particleOwner == null) return; // Instancing not yet set-up by particle system owner
		if (this.particleSystems != null && meshContext) {
			if (this.particleChildren == null) {
				this.particleChildren = [];
				for (let psys of this.particleSystems) {
					// let c: MeshObject = Scene.getChild(psys.data.raw.instance_object);
					Scene.spawnObject(psys.data.instance_object, null, (o: BaseObject) => {
						if (o != null) {
							let c: MeshObject = o.ext;
							this.particleChildren.push(c);
							c.particleOwner = this;
							c.particleIndex = this.particleChildren.length - 1;
						}
					});
				}
			}
			for (let i = 0; i < this.particleSystems.length; ++i) {
				ParticleSystem.update(this.particleSystems[i], this.particleChildren[i], this);
			}
		}
		if (this.particleSystems != null && this.particleSystems.length > 0 && !this.base.raw.render_emitter) return;
		///end

		if (this.cullMaterial(context)) return;

		// Get context
		let materialContexts: TMaterialContext[] = [];
		let shaderContexts: TShaderContext[] = [];
		this.getContexts(context, this.materials, materialContexts, shaderContexts);

		Uniforms.posUnpack = this.data.scale_pos;
		Uniforms.texUnpack = this.data.scale_tex;
		this.base.transform.update();

		// Render mesh
		for (let i = 0; i < this.data._indexBuffers.length; ++i) {

			let mi = this.data._materialIndices[i];
			if (shaderContexts.length <= mi || shaderContexts[mi] == null) continue;
			this.materialIndex = mi;

			// Check context skip
			if (this.materials.length > mi && this.skipContext(context, this.materials[mi])) continue;

			let scontext = shaderContexts[mi];
			if (scontext == null) continue;
			let elems = scontext.vertex_elements;

			// Uniforms
			if (scontext._pipeState != MeshObject.lastPipeline) {
				g.setPipeline(scontext._pipeState);
				MeshObject.lastPipeline = scontext._pipeState;
				// Uniforms.setContextConstants(g, scontext, bindParams);
			}
			Uniforms.setContextConstants(g, scontext, bindParams); //
			Uniforms.setObjectConstants(g, scontext, this.base);
			if (materialContexts.length > mi) {
				Uniforms.setMaterialConstants(g, scontext, materialContexts[mi]);
			}

			// VB / IB
			if (this.data._instancedVB != null) {
				g.setVertexBuffers([MeshData.get(this.data, elems), this.data._instancedVB]);
			}
			else {
				g.setVertexBuffer(MeshData.get(this.data, elems));
			}

			g.setIndexBuffer(this.data._indexBuffers[i]);

			// Draw
			if (this.data._instanced) {
				g.drawIndexedVerticesInstanced(this.data._instanceCount, 0, -1);
			}
			else {
				g.drawIndexedVertices(0, -1);
			}
		}

		this.prevMatrix.setFrom(this.base.transform.worldUnpack);
	}

	validContext = (mats: TMaterialData[], context: string): bool => {
		for (let mat of mats) if (MaterialData.getContext(mat, context) != null) return true;
		return false;
	}

	computeCameraDistance = (camX: f32, camY: f32, camZ: f32) => {
		// Render path mesh sorting
		this.cameraDistance = Vec4.distancef(camX, camY, camZ, this.base.transform.worldx(), this.base.transform.worldy(), this.base.transform.worldz());
	}

	computeScreenSize = (camera: CameraObject) => {
		// Approx..
		// let rp = camera.renderPath;
		// let screenVolume = rp.currentW * rp.currentH;
		let tr = this.base.transform;
		let volume = tr.dim.x * tr.dim.y * tr.dim.z;
		this.screenSize = volume * (1.0 / this.cameraDistance);
		this.screenSize = this.screenSize > 1.0 ? 1.0 : this.screenSize;
	}
}
