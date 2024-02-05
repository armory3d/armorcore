
class TMeshObject {
	base: TBaseObject;
	data: mesh_data_t = null;
	materials: material_data_t[];
	materialIndex = 0;
	///if arm_particles
	particleSystems: particle_sys_t[] = null; // Particle owner
	particleChildren: TMeshObject[] = null;
	particleOwner: TMeshObject = null; // Particle object
	particleIndex = -1;
	///end
	cameraDistance: f32;
	screenSize = 0.0;
	frustumCulling = true;
	skip_context: string = null; // Do not draw this context
	force_context: string = null; // Draw only this context
	prevMatrix = mat4_identity();
}

class MeshObject {
	static lastPipeline: pipeline_t = null;

	static create(data: mesh_data_t, materials: material_data_t[]): TMeshObject {
		let raw = new TMeshObject();
		raw.base = BaseObject.create();
		raw.base.ext = raw;

		raw.materials = materials;
		MeshObject.setData(raw, data);
		scene_meshes.push(raw);
		return raw;
	}

	static setData = (raw: TMeshObject, data: mesh_data_t) => {
		raw.data = data;
		data._refcount++;
		MeshData.build(data);

		// Scale-up packed (-1,1) mesh coords
		raw.base.transform.scale_world = data.scale_pos;
	}

	static remove = (raw: TMeshObject) => {
		///if arm_particles
		if (raw.particleChildren != null) {
			for (let c of raw.particleChildren) MeshObject.remove(c);
			raw.particleChildren = null;
		}
		if (raw.particleSystems != null) {
			for (let psys of raw.particleSystems) particle_sys_remove(psys);
			raw.particleSystems = null;
		}
		///end
		array_remove(scene_meshes, raw);
		raw.data._refcount--;

		BaseObject.removeSuper(raw.base);
	}

	static setupAnimation = (raw: TMeshObject, oactions: scene_t[] = null) => {
		///if arm_skin
		let hasAction = raw.base.parent != null && raw.base.parent.raw != null && raw.base.parent.raw.bone_actions != null;
		if (hasAction) {
			let armatureName = raw.base.parent.name;
			raw.base.animation = BaseObject.getParentArmature(raw.base, armatureName).base;
			if (raw.base.animation == null) raw.base.animation = BoneAnimation.create(armatureName).base;
			if (raw.data.skin != null) BoneAnimation.setSkin(raw.base.animation.ext, raw);
		}
		///end

		BaseObject.setupAnimationSuper(raw.base, oactions);
	}

	///if arm_particles
	static setupParticleSystem = (raw: TMeshObject, sceneName: string, pref: particle_ref_t) => {
		if (raw.particleSystems == null) raw.particleSystems = [];
		let psys = particle_sys_create(sceneName, pref);
		raw.particleSystems.push(psys);
	}
	///end

	static setCulled = (raw: TMeshObject, b: bool): bool => {
		raw.base.culled = b;
		return b;
	}

	static cullMaterial = (raw: TMeshObject, context: string): bool => {
		// Skip render if material does not contain current context
		let mats = raw.materials;
		if (!MeshObject.validContext(raw, mats, context)) return true;

		if (!raw.base.visible) return MeshObject.setCulled(raw, true);

		if (raw.skip_context == context) return MeshObject.setCulled(raw, true);
		if (raw.force_context != null && raw.force_context != context) return MeshObject.setCulled(raw, true);

		return MeshObject.setCulled(raw, false);
	}

	static cullMesh = (raw: TMeshObject, context: string, camera: TCameraObject, light: TLightObject): bool => {
		if (camera == null) return false;

		if (camera.data.frustum_culling && raw.frustumCulling) {
			// Scale radius for skinned mesh and particle system
			// TODO: define skin & particle bounds
			let radiusScale = raw.data.skin != null ? 2.0 : 1.0;
			///if arm_particles
			// particleSystems for update, particleOwner for render
			if (raw.particleSystems != null || raw.particleOwner != null) radiusScale *= 1000;
			///end
			if (context == "voxel") radiusScale *= 100;
			if (raw.data._instanced) radiusScale *= 100;
			let frustumPlanes = camera.frustumPlanes;

			if (!CameraObject.sphereInFrustum(frustumPlanes, raw.base.transform, radiusScale)) {
				return MeshObject.setCulled(raw, true);
			}
		}

		raw.base.culled = false;
		return raw.base.culled;
	}

	static skipContext = (raw: TMeshObject, context: string, mat: material_data_t): bool => {
		if (mat.skip_context != null &&
			mat.skip_context == context) {
			return true;
		}
		return false;
	}

	static getContexts = (raw: TMeshObject, context: string, materials: material_data_t[], materialContexts: material_context_t[], shaderContexts: shader_context_t[]) => {
		for (let mat of materials) {
			let found = false;
			for (let i = 0; i < mat.contexts.length; ++i) {
				if (mat.contexts[i].name.substr(0, context.length) == context) {
					materialContexts.push(mat._contexts[i]);
					shaderContexts.push(shader_data_get_context(mat._shader, context));
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

	static render = (raw: TMeshObject, g: g4_t, context: string, bindParams: string[]) => {
		if (raw.data == null || !raw.data._ready) return; // Data not yet streamed
		if (!raw.base.visible) return; // Skip render if object is hidden
		if (MeshObject.cullMesh(raw, context, scene_camera,_render_path_light)) return;
		let meshContext = raw.base.raw != null ? context == "mesh" : false;

		///if arm_particles
		if (raw.base.raw != null && raw.base.raw.is_particle && raw.particleOwner == null) return; // Instancing not yet set-up by particle system owner
		if (raw.particleSystems != null && meshContext) {
			if (raw.particleChildren == null) {
				raw.particleChildren = [];
				for (let psys of raw.particleSystems) {
					// let c: TMeshObject = scene_get_child(psys.data.raw.instance_object);
					scene_spawn_object(psys.data.instance_object, null, (o: TBaseObject) => {
						if (o != null) {
							let c: TMeshObject = o.ext;
							raw.particleChildren.push(c);
							c.particleOwner = raw;
							c.particleIndex = raw.particleChildren.length - 1;
						}
					});
				}
			}
			for (let i = 0; i < raw.particleSystems.length; ++i) {
				particle_sys_update(raw.particleSystems[i], raw.particleChildren[i], raw);
			}
		}
		if (raw.particleSystems != null && raw.particleSystems.length > 0 && !raw.base.raw.render_emitter) return;
		///end

		if (MeshObject.cullMaterial(raw, context)) return;

		// Get context
		let materialContexts: material_context_t[] = [];
		let shaderContexts: shader_context_t[] = [];
		MeshObject.getContexts(raw, context, raw.materials, materialContexts, shaderContexts);

		uniforms_pos_unpack = raw.data.scale_pos;
		uniforms_tex_unpack = raw.data.scale_tex;
		transform_update(raw.base.transform);

		// Render mesh
		for (let i = 0; i < raw.data._index_buffers.length; ++i) {

			let mi = raw.data._material_indices[i];
			if (shaderContexts.length <= mi || shaderContexts[mi] == null) continue;
			raw.materialIndex = mi;

			// Check context skip
			if (raw.materials.length > mi && MeshObject.skipContext(raw, context, raw.materials[mi])) continue;

			let scontext = shaderContexts[mi];
			if (scontext == null) continue;
			let elems = scontext.vertex_elements;

			// Uniforms
			if (scontext._pipe_state != MeshObject.lastPipeline) {
				g4_set_pipeline(scontext._pipe_state);
				MeshObject.lastPipeline = scontext._pipe_state;
				// setContextConstants(g, scontext, bindParams);
			}
			uniforms_set_context_consts(g, scontext, bindParams); //
			uniforms_set_obj_consts(g, scontext, raw.base);
			if (materialContexts.length > mi) {
				uniforms_set_material_consts(g, scontext, materialContexts[mi]);
			}

			// VB / IB
			if (raw.data._instanced_vb != null) {
				g4_set_vertex_buffers([MeshData.get(raw.data, elems), raw.data._instanced_vb]);
			}
			else {
				g4_set_vertex_buffer(MeshData.get(raw.data, elems));
			}

			g4_set_index_buffer(raw.data._index_buffers[i]);

			// Draw
			if (raw.data._instanced) {
				g4_draw_inst(raw.data._instance_count, 0, -1);
			}
			else {
				g4_draw(0, -1);
			}
		}

		mat4_set_from(raw.prevMatrix, raw.base.transform.world_unpack);
	}

	static validContext = (raw: TMeshObject, mats: material_data_t[], context: string): bool => {
		for (let mat of mats) if (MaterialData.getContext(mat, context) != null) return true;
		return false;
	}

	static computeCameraDistance = (raw: TMeshObject, camX: f32, camY: f32, camZ: f32) => {
		// Render path mesh sorting
		raw.cameraDistance = vec4_dist_f(camX, camY, camZ, transform_world_x(raw.base.transform), transform_world_y(raw.base.transform), transform_world_z(raw.base.transform));
	}

	static computeScreenSize = (raw: TMeshObject, camera: TCameraObject) => {
		// Approx..
		// let rp = camera.renderPath;
		// let screenVolume = rp.currentW * rp.currentH;
		let tr = raw.base.transform;
		let volume = tr.dim.x * tr.dim.y * tr.dim.z;
		raw.screenSize = volume * (1.0 / raw.cameraDistance);
		raw.screenSize = raw.screenSize > 1.0 ? 1.0 : raw.screenSize;
	}
}
