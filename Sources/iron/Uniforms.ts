/// <reference path='./Vec4.ts'/>

// Structure for setting shader uniforms
class Uniforms {

	static helpMat = Mat4.identity();
	static helpMat2 = Mat4.identity();
	static helpMat3 = Mat3.identity();
	static helpVec = new Vec4();
	static helpVec2 = new Vec4();
	static helpQuat = new Quat(); // Keep at identity

	static externalTextureLinks: ((o: BaseObject, md: MaterialData, s: string)=>Image)[] = null;
	static externalMat4Links: ((o: BaseObject, md: MaterialData, s: string)=>Mat4)[] = null;
	static externalVec4Links: ((o: BaseObject, md: MaterialData, s: string)=>Vec4)[] = null;
	static externalVec3Links: ((o: BaseObject, md: MaterialData, s: string)=>Vec4)[] = null;
	static externalVec2Links: ((o: BaseObject, md: MaterialData, s: string)=>Vec4)[] = null;
	static externalFloatLinks: ((o: BaseObject, md: MaterialData, s: string)=>Null<f32>)[] = null;
	static externalFloatsLinks: ((o: BaseObject, md: MaterialData, s: string)=>Float32Array)[] = null;
	static externalIntLinks: ((o: BaseObject, md: MaterialData, s: string)=>Null<i32>)[] = null;
	static posUnpack: Null<f32> = null;
	static texUnpack: Null<f32> = null;

	static setContextConstants = (g: Graphics4, context: ShaderContext, bindParams: string[]) => {
		if (context.raw.constants != null) {
			for (let i = 0; i < context.raw.constants.length; ++i) {
				let c = context.raw.constants[i];
				Uniforms.setContextConstant(g, context.constants[i], c);
			}
		}

		// Texture context constants
		if (bindParams != null) { // Bind targets
			for (let i = 0; i < Math.floor(bindParams.length / 2); ++i) {
				let pos = i * 2; // bind params = [texture, samplerID]
				let rtID = bindParams[pos];
				let samplerID = bindParams[pos + 1];
				let attachDepth = false; // Attach texture depth if '_' is prepended
				let char = rtID.charAt(0);
				if (char == "_") {
					attachDepth = true;
					rtID = rtID.substr(1);
				}
				let rt = attachDepth ? RenderPath.depthToRenderTarget.get(rtID) : RenderPath.renderTargets.get(rtID);
				Uniforms.bindRenderTarget(g, rt, context, samplerID, attachDepth);
			}
		}

		// Texture links
		if (context.raw.texture_units != null) {
			for (let j = 0; j < context.raw.texture_units.length; ++j) {
				let tulink = context.raw.texture_units[j].link;
				if (tulink == null) continue;

				if (tulink.charAt(0) == "$") { // Link to embedded data
					g.setTexture(context.textureUnits[j], Scene.embedded.get(tulink.substr(1)));
					if (tulink.endsWith(".raw")) { // Raw 3D texture
						g.setTexture3DParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
					}
					else { // 2D texture
						g.setTextureParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
					}
				}
				else {
					switch (tulink) {
						case "_envmapRadiance": {
							let w = Scene.world;
							if (w != null) {
								g.setTexture(context.textureUnits[j], w.radiance);
								g.setTextureParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
							}
							break;
						}
						case "_envmap": {
							let w = Scene.world;
							if (w != null) {
								g.setTexture(context.textureUnits[j], w.envmap);
								g.setTextureParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
							}
							break;
						}
					}
				}
			}
		}
	}

	static setObjectConstants = (g: Graphics4, context: ShaderContext, object: BaseObject) => {
		if (context.raw.constants != null) {
			for (let i = 0; i < context.raw.constants.length; ++i) {
				let c = context.raw.constants[i];
				Uniforms.setObjectConstant(g, object, context.constants[i], c);
			}
		}

		// Texture object constants
		// External
		if (Uniforms.externalTextureLinks != null) {
			if (context.raw.texture_units != null) {
				for (let j = 0; j < context.raw.texture_units.length; ++j) {
					let tu = context.raw.texture_units[j];
					if (tu.link == null) continue;
					let tuAddrU = Uniforms.getTextureAddressing(tu.addressing_u);
					let tuAddrV = Uniforms.getTextureAddressing(tu.addressing_v);
					let tuFilterMin = Uniforms.getTextureFilter(tu.filter_min);
					let tuFilterMag = Uniforms.getTextureFilter(tu.filter_mag);
					let tuMipMapFilter = Uniforms.getMipMapFilter(tu.mipmap_filter);

					for (let f of Uniforms.externalTextureLinks) {
						let image = f(object, Uniforms.currentMat(object), tu.link);
						if (image != null) {
							tu.link.endsWith("_depth") ?
								g.setTextureDepth(context.textureUnits[j], image) :
								g.setTexture(context.textureUnits[j], image);
							g.setTextureParameters(context.textureUnits[j], tuAddrU, tuAddrV, tuFilterMin, tuFilterMag, tuMipMapFilter);
							break;
						}
					}
				}
			}
		}
	}

	static bindRenderTarget = (g: Graphics4, rt: RenderTarget, context: ShaderContext, samplerID: string, attachDepth: bool) => {
		if (rt != null) {
			let tus = context.raw.texture_units;

			for (let j = 0; j < tus.length; ++j) { // Set texture
				if (samplerID == tus[j].name) {
					let isImage = tus[j].is_image != null && tus[j].is_image;
					let paramsSet = false;

					if (rt.raw.depth > 1) { // sampler3D
						g.setTexture3DParameters(context.textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.AnisotropicFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}

					if (isImage) {
						g.setImageTexture(context.textureUnits[j], rt.image); // image2D/3D
						// Multiple voxel volumes, always set params
						g.setTexture3DParameters(context.textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.PointFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}
					else {
						if (attachDepth) g.setTextureDepth(context.textureUnits[j], rt.image); // sampler2D
						else g.setTexture(context.textureUnits[j], rt.image); // sampler2D
					}

					if (!paramsSet && rt.raw.mipmaps != null && rt.raw.mipmaps == true && !isImage) {
						g.setTextureParameters(context.textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}

					if (!paramsSet) {
						if (rt.raw.name.startsWith("bloom")) {
							// Use bilinear filter for bloom mips to get correct blur
							g.setTextureParameters(context.textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
							paramsSet = true;
						}
						if (attachDepth) {
							g.setTextureParameters(context.textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.PointFilter, TextureFilter.PointFilter, MipMapFilter.NoMipFilter);
							paramsSet = true;
						}
					}

					if (!paramsSet) {
						// No filtering when sampling render targets
						let oc = context.overrideContext;
						let allowParams = oc == null || oc.shared_sampler == null || oc.shared_sampler == samplerID;
						if (allowParams) {
							let addressing = (oc != null && oc.addressing == "repeat") ? TextureAddressing.Repeat : TextureAddressing.Clamp;
							let filter = (oc != null && oc.filter == "point") ? TextureFilter.PointFilter : TextureFilter.LinearFilter;
							g.setTextureParameters(context.textureUnits[j], addressing, addressing, filter, filter, MipMapFilter.NoMipFilter);
						}
						paramsSet = true;
					}
				}
			}
		}
	}

	static setContextConstant = (g: Graphics4, location: ConstantLocation, c: TShaderConstant): bool => {
		if (c.link == null) return true;

		let camera = Scene.camera;
		let light = RenderPath.light;

		if (c.type == "mat4") {
			let m: Mat4 = null;
			switch (c.link) {
				case "_viewMatrix": {
					m = camera.V;
					break;
				}
				case "_projectionMatrix": {
					m = camera.P;
					break;
				}
				case "_inverseProjectionMatrix": {
					Uniforms.helpMat.getInverse(camera.P);
					m = Uniforms.helpMat;
					break;
				}
				case "_viewProjectionMatrix": {
					m = camera.VP;
					break;
				}
				case "_inverseViewProjectionMatrix": {
					Uniforms.helpMat.setFrom(camera.V);
					Uniforms.helpMat.multmat(camera.P);
					Uniforms.helpMat.getInverse(Uniforms.helpMat);
					m = Uniforms.helpMat;
					break;
				}
				case "_skydomeMatrix": {
					let tr = camera.transform;
					Uniforms.helpVec.set(tr.worldx(), tr.worldy(), tr.worldz() - 3.5); // Sky
					let bounds = camera.data.far_plane * 0.95;
					Uniforms.helpVec2.set(bounds, bounds, bounds);
					Uniforms.helpMat.compose(Uniforms.helpVec, Uniforms.helpQuat, Uniforms.helpVec2);
					Uniforms.helpMat.multmat(camera.V);
					Uniforms.helpMat.multmat(camera.P);
					m = Uniforms.helpMat;
					break;
				}
				default: // Unknown uniform
					return false;
			}

			g.setMatrix(location, m != null ? m : Mat4.identity());
			return true;
		}
		else if (c.type == "vec4") {
			let v: Vec4 = null;
			Uniforms.helpVec.set(0, 0, 0, 0);
			switch (c.link) {
				default:
					return false;
			}

			if (v != null) {
				g.setFloat4(location, v.x, v.y, v.z, v.w);
			}
			else {
				g.setFloat4(location, 0, 0, 0, 0);
			}
			return true;
		}
		else if (c.type == "vec3") {
			let v: Vec4 = null;
			Uniforms.helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_lightDirection": {
					if (light != null) {
						Uniforms.helpVec = light.look().normalize();
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_pointPosition": {
					let point = RenderPath.point;
					if (point != null) {
						Uniforms.helpVec.set(point.transform.worldx(), point.transform.worldy(), point.transform.worldz());
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_pointColor": {
					let point = RenderPath.point;
					if (point != null) {
						let str = point.visible ? point.data.strength : 0.0;
						Uniforms.helpVec.set(point.data.color[0] * str, point.data.color[1] * str, point.data.color[2] * str);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea0": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Uniforms.helpVec.set(-sx, sy, 0.0);
						Uniforms.helpVec.applymat(light.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea1": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Uniforms.helpVec.set(sx, sy, 0.0);
						Uniforms.helpVec.applymat(light.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea2": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Uniforms.helpVec.set(sx, -sy, 0.0);
						Uniforms.helpVec.applymat(light.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea3": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Uniforms.helpVec.set(-sx, -sy, 0.0);
						Uniforms.helpVec.applymat(light.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_cameraPosition": {
					Uniforms.helpVec.set(camera.transform.worldx(), camera.transform.worldy(), camera.transform.worldz());
					v = Uniforms.helpVec;
					break;
				}
				case "_cameraLook": {
					Uniforms.helpVec = camera.lookWorld().normalize();
					v = Uniforms.helpVec;
					break;
				}
				default:
					return false;
			}

			if (v != null) {
				g.setFloat3(location, v.x, v.y, v.z);
			}
			else {
				g.setFloat3(location, 0.0, 0.0, 0.0);
			}
			return true;
		}
		else if (c.type == "vec2") {
			let v: Vec4 = null;
			Uniforms.helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_vec2x": {
					v = Uniforms.helpVec;
					v.x = 1.0;
					v.y = 0.0;
					break;
				}
				case "_vec2xInv": {
					v = Uniforms.helpVec;
					v.x = 1.0 / RenderPath.currentW;
					v.y = 0.0;
					break;
				}
				case "_vec2x2": {
					v = Uniforms.helpVec;
					v.x = 2.0;
					v.y = 0.0;
					break;
				}
				case "_vec2x2Inv": {
					v = Uniforms.helpVec;
					v.x = 2.0 / RenderPath.currentW;
					v.y = 0.0;
					break;
				}
				case "_vec2y": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 1.0;
					break;
				}
				case "_vec2yInv": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 1.0 / RenderPath.currentH;
					break;
				}
				case "_vec2y2": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 2.0;
					break;
				}
				case "_vec2y2Inv": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 2.0 / RenderPath.currentH;
					break;
				}
				case "_vec2y3": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 3.0;
					break;
				}
				case "_vec2y3Inv": {
					v = Uniforms.helpVec;
					v.x = 0.0;
					v.y = 3.0 / RenderPath.currentH;
					break;
				}
				case "_screenSize": {
					v = Uniforms.helpVec;
					v.x = RenderPath.currentW;
					v.y = RenderPath.currentH;
					break;
				}
				case "_screenSizeInv": {
					v = Uniforms.helpVec;
					v.x = 1.0 / RenderPath.currentW;
					v.y = 1.0 / RenderPath.currentH;
					break;
				}
				case "_cameraPlaneProj": {
					let near = camera.data.near_plane;
					let far = camera.data.far_plane;
					v = Uniforms.helpVec;
					v.x = far / (far - near);
					v.y = (-far * near) / (far - near);
					break;
				}
				default:
					return false;
			}

			if (v != null) {
				g.setFloat2(location, v.x, v.y);
			}
			else {
				g.setFloat2(location, 0.0, 0.0);
			}
			return true;
		}
		else if (c.type == "float") {
			let f: Null<f32> = null;
			switch (c.link) {
				case "_time": {
					f = Time.time();
					break;
				}
				case "_aspectRatioWindowF": {
					f = App.w() / App.h();
					break;
				}
				default:
					return false;
			}

			g.setFloat(location, f != null ? f : 0);
			return true;
		}
		else if (c.type == "floats") {
			let fa: Float32Array = null;
			switch (c.link) {
				case "_envmapIrradiance": {
					fa = Scene.world == null ? WorldData.getEmptyIrradiance() : Scene.world.irradiance;
					break;
				}
			}

			if (fa != null) {
				g.setFloats(location, fa);
				return true;
			}
		}
		else if (c.type == "int") {
			let i: Null<i32> = null;
			switch (c.link) {
				case "_envmapNumMipmaps": {
					let w = Scene.world;
					i = w != null ? w.raw.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
					break;
				}
				default:
					return false;
			}

			g.setInt(location, i != null ? i : 0);
			return true;
		}
		return false;
	}

	static setObjectConstant = (g: Graphics4, object: BaseObject, location: ConstantLocation, c: TShaderConstant) => {
		if (c.link == null) return;

		let camera = Scene.camera;
		let light = RenderPath.light;

		if (c.type == "mat4") {
			let m: Mat4 = null;
			switch (c.link) {
				case "_worldMatrix": {
					m = object.transform.worldUnpack;
					break;
				}
				case "_inverseWorldMatrix": {
					Uniforms.helpMat.getInverse(object.transform.worldUnpack);
					m = Uniforms.helpMat;
					break;
				}
				case "_worldViewProjectionMatrix": {
					Uniforms.helpMat.setFrom(object.transform.worldUnpack);
					Uniforms.helpMat.multmat(camera.V);
					Uniforms.helpMat.multmat(camera.P);
					m = Uniforms.helpMat;
					break;
				}
				case "_worldViewMatrix": {
					Uniforms.helpMat.setFrom(object.transform.worldUnpack);
					Uniforms.helpMat.multmat(camera.V);
					m = Uniforms.helpMat;
					break;
				}
				case "_prevWorldViewProjectionMatrix": {
					Uniforms.helpMat.setFrom((object as MeshObject).prevMatrix);
					Uniforms.helpMat.multmat(camera.prevV);
					// helpMat.multmat(camera.prevP);
					Uniforms.helpMat.multmat(camera.P);
					m = Uniforms.helpMat;
					break;
				}
				///if arm_particles
				case "_particleData": {
					let mo = object as MeshObject;
					if (mo.particleOwner != null && mo.particleOwner.particleSystems != null) {
						m = mo.particleOwner.particleSystems[mo.particleIndex].getData();
					}
					break;
				}
				///end
			}

			if (m == null && Uniforms.externalMat4Links != null) {
				for (let fn of Uniforms.externalMat4Links) {
					m = fn(object, Uniforms.currentMat(object), c.link);
					if (m != null) break;
				}
			}

			if (m == null) return;
			g.setMatrix(location, m);
		}
		else if (c.type == "mat3") {
			let m: Mat3 = null;
			switch (c.link) {
				case "_normalMatrix": {
					Uniforms.helpMat.getInverse(object.transform.world);
					Uniforms.helpMat.transpose3x3();
					Uniforms.helpMat3.setFrom4(Uniforms.helpMat);
					m = Uniforms.helpMat3;
					break;
				}
				case "_viewMatrix3": {
					Uniforms.helpMat3.setFrom4(camera.V);
					m = Uniforms.helpMat3;
					break;
				}
			}

			if (m == null) return;
			g.setMatrix3(location, m);
		}
		else if (c.type == "vec4") {
			let v: Vec4 = null;
			Uniforms.helpVec.set(0, 0, 0);

			if (v == null && Uniforms.externalVec4Links != null) {
				for (let fn of Uniforms.externalVec4Links) {
					v = fn(object, Uniforms.currentMat(object), c.link);
					if (v != null) break;
				}
			}

			if (v == null) return;
			g.setFloat4(location, v.x, v.y, v.z, v.w);
		}
		else if (c.type == "vec3") {
			let v: Vec4 = null;
			Uniforms.helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_dim": { // Model space
					let d = object.transform.dim;
					let s = object.transform.scale;
					Uniforms.helpVec.set((d.x / s.x), (d.y / s.y), (d.z / s.z));
					v = Uniforms.helpVec;
					break;
				}
				case "_halfDim": { // Model space
					let d = object.transform.dim;
					let s = object.transform.scale;
					Uniforms.helpVec.set((d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
					v = Uniforms.helpVec;
					break;
				}
			}

			if (v == null && Uniforms.externalVec3Links != null) {
				for (let f of Uniforms.externalVec3Links) {
					v = f(object, Uniforms.currentMat(object), c.link);
					if (v != null) break;
				}
			}

			if (v == null) return;
			g.setFloat3(location, v.x, v.y, v.z);
		}
		else if (c.type == "vec2") {
			let vx: Null<f32> = null;
			let vy: Null<f32> = null;

			if (vx == null && Uniforms.externalVec2Links != null) {
				for (let fn of Uniforms.externalVec2Links) {
					let v = fn(object, Uniforms.currentMat(object), c.link);
					if (v != null) {
						vx = v.x;
						vy = v.y;
						break;
					}
				}
			}

			if (vx == null) return;
			g.setFloat2(location, vx, vy);
		}
		else if (c.type == "float") {
			let f: Null<f32> = null;
			switch (c.link) {
				case "_objectInfoIndex": {
					f = object.uid;
					break;
				}
				case "_objectInfoMaterialIndex": {
					f = Uniforms.currentMat(object).uid;
					break;
				}
				case "_objectInfoRandom": {
					f = object.urandom;
					break;
				}
				case "_posUnpack": {
					f = Uniforms.posUnpack != null ? Uniforms.posUnpack : 1.0;
					break;
				}
				case "_texUnpack": {
					f = Uniforms.texUnpack != null ? Uniforms.texUnpack : 1.0;
					break;
				}
			}

			if (f == null && Uniforms.externalFloatLinks != null) {
				for (let fn of Uniforms.externalFloatLinks) {
					let res = fn(object, Uniforms.currentMat(object), c.link);
					if (res != null) {
						f = res;
						break;
					}
				}
			}

			if (f == null) return;
			g.setFloat(location, f);
		}
		else if (c.type == "floats") {
			let fa: Float32Array = null;
			switch (c.link) {
				///if arm_skin
				case "_skinBones": {
					if (object.animation != null) {
						fa = (object.animation as BoneAnimation).skinBuffer;
					}
					break;
				}
				///end
			}

			if (fa == null && Uniforms.externalFloatsLinks != null) {
				for (let fn of Uniforms.externalFloatsLinks) {
					fa = fn(object, Uniforms.currentMat(object), c.link);
					if (fa != null) break;
				}
			}

			if (fa == null) return;
			g.setFloats(location, fa);
		}
		else if (c.type == "int") {
			let i: Null<i32> = null;
			switch (c.link) {
				case "_uid": {
					i = object.uid;
					break;
				}
			}

			if (i == null && Uniforms.externalIntLinks != null) {
				for (let fn of Uniforms.externalIntLinks) {
					let res = fn(object, Uniforms.currentMat(object), c.link);
					if (res != null) {
						i = res;
						break;
					}
				}
			}

			if (i == null) return;
			g.setInt(location, i);
		}
	}

	static setMaterialConstants = (g: Graphics4, context: ShaderContext, materialContext: MaterialContext) => {
		if (materialContext.raw.bind_constants != null) {
			for (let i = 0; i < materialContext.raw.bind_constants.length; ++i) {
				let matc = materialContext.raw.bind_constants[i];
				let pos = -1;
				for (let i = 0; i < context.raw.constants.length; ++i) {
					if (context.raw.constants[i].name == matc.name) {
						pos = i;
						break;
					}
				}
				if (pos == -1) continue;
				let c = context.raw.constants[pos];

				Uniforms.setMaterialConstant(g, context.constants[pos], c, matc);
			}
		}

		if (materialContext.textures != null) {
			for (let i = 0; i < materialContext.textures.length; ++i) {
				let mname = materialContext.raw.bind_textures[i].name;

				for (let j = 0; j < context.textureUnits.length; ++j) {
					let sname = context.raw.texture_units[j].name;
					if (mname == sname) {
						g.setTexture(context.textureUnits[j], materialContext.textures[i]);
						// After texture sampler have been assigned, set texture parameters
						materialContext.setTextureParameters(g, i, context, j);
						break;
					}
				}
			}
		}
	}

	static currentMat = (object: BaseObject): MaterialData => {
		if (object != null && object.constructor == MeshObject) {
			let mo = object as MeshObject;
			return mo.materials[mo.materialIndex];
		}
		return null;
	}

	static setMaterialConstant = (g: Graphics4, location: ConstantLocation, c: TShaderConstant, matc: TBindConstant) => {
		switch (c.type) {
			case "vec4":
				g.setFloat4(location, matc.vec4[0], matc.vec4[1], matc.vec4[2], matc.vec4[3]);
				break;
			case "vec3":
				g.setFloat3(location, matc.vec3[0], matc.vec3[1], matc.vec3[2]);
				break;
			case "vec2":
				g.setFloat2(location, matc.vec2[0], matc.vec2[1]);
				break;
			case "float":
				g.setFloat(location,  matc.float);
				break;
			case "bool":
				g.setBool(location, matc.bool);
				break;
			case "int":
				g.setInt(location, matc.int);
				break;
		}
	}

	static getTextureAddressing = (s: string): TextureAddressing => {
		switch (s) {
			case "clamp": return TextureAddressing.Clamp;
			case "mirror": return TextureAddressing.Mirror;
			default: return TextureAddressing.Repeat;
		}
	}

	static getTextureFilter = (s: string): TextureFilter => {
		switch (s) {
			case "anisotropic": return TextureFilter.AnisotropicFilter;
			case "point": return TextureFilter.PointFilter;
			default: return TextureFilter.LinearFilter;
		}
	}

	static getMipMapFilter = (s: string): MipMapFilter => {
		switch (s) {
			case "linear": return MipMapFilter.LinearMipFilter;
			case "point": return MipMapFilter.PointMipFilter;
			default: return MipMapFilter.NoMipFilter;
		}
	}
}
