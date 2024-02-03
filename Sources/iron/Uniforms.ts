/// <reference path='./Vec4.ts'/>

// Structure for setting shader uniforms
class Uniforms {

	static helpMat = Mat4.identity();
	static helpMat2 = Mat4.identity();
	static helpMat3 = mat3_identity();
	static helpVec = Vec4.create();
	static helpVec2 = Vec4.create();
	static helpQuat = Quat.create(); // Keep at identity

	static externalTextureLinks: ((o: TBaseObject, md: TMaterialData, s: string)=>ImageRaw)[] = null;
	static externalMat4Links: ((o: TBaseObject, md: TMaterialData, s: string)=>TMat4)[] = null;
	static externalVec4Links: ((o: TBaseObject, md: TMaterialData, s: string)=>TVec4)[] = null;
	static externalVec3Links: ((o: TBaseObject, md: TMaterialData, s: string)=>TVec4)[] = null;
	static externalVec2Links: ((o: TBaseObject, md: TMaterialData, s: string)=>TVec4)[] = null;
	static externalFloatLinks: ((o: TBaseObject, md: TMaterialData, s: string)=>Null<f32>)[] = null;
	static externalFloatsLinks: ((o: TBaseObject, md: TMaterialData, s: string)=>Float32Array)[] = null;
	static externalIntLinks: ((o: TBaseObject, md: TMaterialData, s: string)=>Null<i32>)[] = null;
	static posUnpack: Null<f32> = null;
	static texUnpack: Null<f32> = null;

	static setContextConstants = (g: Graphics4Raw, context: TShaderContext, bindParams: string[]) => {
		if (context.constants != null) {
			for (let i = 0; i < context.constants.length; ++i) {
				let c = context.constants[i];
				Uniforms.setContextConstant(g, context._constants[i], c);
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
		if (context.texture_units != null) {
			for (let j = 0; j < context.texture_units.length; ++j) {
				let tulink = context.texture_units[j].link;
				if (tulink == null) continue;

				if (tulink.charAt(0) == "$") { // Link to embedded data
					Graphics4.setTexture(context._textureUnits[j], Scene.embedded.get(tulink.substr(1)));
					if (tulink.endsWith(".raw")) { // Raw 3D texture
						Graphics4.setTexture3DParameters(context._textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
					}
					else { // 2D texture
						Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
					}
				}
				else {
					switch (tulink) {
						case "_envmapRadiance": {
							let w = Scene.world;
							if (w != null) {
								Graphics4.setTexture(context._textureUnits[j], w._radiance);
								Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
							}
							break;
						}
						case "_envmap": {
							let w = Scene.world;
							if (w != null) {
								Graphics4.setTexture(context._textureUnits[j], w._envmap);
								Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
							}
							break;
						}
					}
				}
			}
		}
	}

	static setObjectConstants = (g: Graphics4Raw, context: TShaderContext, object: TBaseObject) => {
		if (context.constants != null) {
			for (let i = 0; i < context.constants.length; ++i) {
				let c = context.constants[i];
				Uniforms.setObjectConstant(g, object, context._constants[i], c);
			}
		}

		// Texture object constants
		// External
		if (Uniforms.externalTextureLinks != null) {
			if (context.texture_units != null) {
				for (let j = 0; j < context.texture_units.length; ++j) {
					let tu = context.texture_units[j];
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
								Graphics4.setTextureDepth(context._textureUnits[j], image) :
								Graphics4.setTexture(context._textureUnits[j], image);
							Graphics4.setTextureParameters(context._textureUnits[j], tuAddrU, tuAddrV, tuFilterMin, tuFilterMag, tuMipMapFilter);
							break;
						}
					}
				}
			}
		}
	}

	static bindRenderTarget = (g: Graphics4Raw, rt: RenderTargetRaw, context: TShaderContext, samplerID: string, attachDepth: bool) => {
		if (rt != null) {
			let tus = context.texture_units;

			for (let j = 0; j < tus.length; ++j) { // Set texture
				if (samplerID == tus[j].name) {
					let isImage = tus[j].is_image != null && tus[j].is_image;
					let paramsSet = false;

					if (rt.depth > 1) { // sampler3D
						Graphics4.setTexture3DParameters(context._textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.AnisotropicFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}

					if (isImage) {
						Graphics4.setImageTexture(context._textureUnits[j], rt.image); // image2D/3D
						// Multiple voxel volumes, always set params
						Graphics4.setTexture3DParameters(context._textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.PointFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}
					else {
						if (attachDepth) Graphics4.setTextureDepth(context._textureUnits[j], rt.image); // sampler2D
						else Graphics4.setTexture(context._textureUnits[j], rt.image); // sampler2D
					}

					if (!paramsSet && rt.mipmaps != null && rt.mipmaps == true && !isImage) {
						Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
						paramsSet = true;
					}

					if (!paramsSet) {
						if (rt.name.startsWith("bloom")) {
							// Use bilinear filter for bloom mips to get correct blur
							Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
							paramsSet = true;
						}
						if (attachDepth) {
							Graphics4.setTextureParameters(context._textureUnits[j], TextureAddressing.Clamp, TextureAddressing.Clamp, TextureFilter.PointFilter, TextureFilter.PointFilter, MipMapFilter.NoMipFilter);
							paramsSet = true;
						}
					}

					if (!paramsSet) {
						// No filtering when sampling render targets
						let oc = context._overrideContext;
						let allowParams = oc == null || oc.shared_sampler == null || oc.shared_sampler == samplerID;
						if (allowParams) {
							let addressing = (oc != null && oc.addressing == "repeat") ? TextureAddressing.Repeat : TextureAddressing.Clamp;
							let filter = (oc != null && oc.filter == "point") ? TextureFilter.PointFilter : TextureFilter.LinearFilter;
							Graphics4.setTextureParameters(context._textureUnits[j], addressing, addressing, filter, filter, MipMapFilter.NoMipFilter);
						}
						paramsSet = true;
					}
				}
			}
		}
	}

	static setContextConstant = (g: Graphics4Raw, location: ConstantLocation, c: TShaderConstant): bool => {
		if (c.link == null) return true;

		let camera = Scene.camera;
		let light = RenderPath.light;

		if (c.type == "mat4") {
			let m: TMat4 = null;
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
					Mat4.getInverse(Uniforms.helpMat, camera.P);
					m = Uniforms.helpMat;
					break;
				}
				case "_viewProjectionMatrix": {
					m = camera.VP;
					break;
				}
				case "_inverseViewProjectionMatrix": {
					Mat4.setFrom(Uniforms.helpMat, camera.V);
					Mat4.multmat(Uniforms.helpMat, camera.P);
					Mat4.getInverse(Uniforms.helpMat, Uniforms.helpMat);
					m = Uniforms.helpMat;
					break;
				}
				case "_skydomeMatrix": {
					let tr = camera.base.transform;
					Vec4.set(Uniforms.helpVec, Transform.worldx(tr), Transform.worldy(tr), Transform.worldz(tr) - 3.5); // Sky
					let bounds = camera.data.far_plane * 0.95;
					Vec4.set(Uniforms.helpVec2, bounds, bounds, bounds);
					Mat4.compose(Uniforms.helpMat, Uniforms.helpVec, Uniforms.helpQuat, Uniforms.helpVec2);
					Mat4.multmat(Uniforms.helpMat, camera.V);
					Mat4.multmat(Uniforms.helpMat, camera.P);
					m = Uniforms.helpMat;
					break;
				}
				default: // Unknown uniform
					return false;
			}

			Graphics4.setMatrix(location, m != null ? m : Mat4.identity());
			return true;
		}
		else if (c.type == "vec4") {
			let v: TVec4 = null;
			Vec4.set(Uniforms.helpVec, 0, 0, 0, 0);
			switch (c.link) {
				default:
					return false;
			}

			if (v != null) {
				Graphics4.setFloat4(location, v.x, v.y, v.z, v.w);
			}
			else {
				Graphics4.setFloat4(location, 0, 0, 0, 0);
			}
			return true;
		}
		else if (c.type == "vec3") {
			let v: TVec4 = null;
			Vec4.set(Uniforms.helpVec, 0, 0, 0);
			switch (c.link) {
				case "_lightDirection": {
					if (light != null) {
						Uniforms.helpVec = Vec4.normalize(LightObject.look(light));
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_pointPosition": {
					let point = RenderPath.point;
					if (point != null) {
						Vec4.set(Uniforms.helpVec, Transform.worldx(point.base.transform), Transform.worldy(point.base.transform), Transform.worldz(point.base.transform));
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_pointColor": {
					let point = RenderPath.point;
					if (point != null) {
						let str = point.base.visible ? point.data.strength : 0.0;
						Vec4.set(Uniforms.helpVec, point.data.color[0] * str, point.data.color[1] * str, point.data.color[2] * str);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea0": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Vec4.set(Uniforms.helpVec, -sx, sy, 0.0);
						Vec4.applymat(Uniforms.helpVec, light.base.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea1": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Vec4.set(Uniforms.helpVec, sx, sy, 0.0);
						Vec4.applymat(Uniforms.helpVec, light.base.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea2": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Vec4.set(Uniforms.helpVec, sx, -sy, 0.0);
						Vec4.applymat(Uniforms.helpVec, light.base.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_lightArea3": {
					if (light != null && light.data.size != null) {
						let f2: f32 = 0.5;
						let sx: f32 = light.data.size * f2;
						let sy: f32 = light.data.size_y * f2;
						Vec4.set(Uniforms.helpVec, -sx, -sy, 0.0);
						Vec4.applymat(Uniforms.helpVec, light.base.transform.world);
						v = Uniforms.helpVec;
						break;
					}
				}
				case "_cameraPosition": {
					Vec4.set(Uniforms.helpVec, Transform.worldx(camera.base.transform), Transform.worldy(camera.base.transform), Transform.worldz(camera.base.transform));
					v = Uniforms.helpVec;
					break;
				}
				case "_cameraLook": {
					Uniforms.helpVec = Vec4.normalize(CameraObject.lookWorld(camera));
					v = Uniforms.helpVec;
					break;
				}
				default:
					return false;
			}

			if (v != null) {
				Graphics4.setFloat3(location, v.x, v.y, v.z);
			}
			else {
				Graphics4.setFloat3(location, 0.0, 0.0, 0.0);
			}
			return true;
		}
		else if (c.type == "vec2") {
			let v: TVec4 = null;
			Vec4.set(Uniforms.helpVec, 0, 0, 0);
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
				Graphics4.setFloat2(location, v.x, v.y);
			}
			else {
				Graphics4.setFloat2(location, 0.0, 0.0);
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

			Graphics4.setFloat(location, f != null ? f : 0);
			return true;
		}
		else if (c.type == "floats") {
			let fa: Float32Array = null;
			switch (c.link) {
				case "_envmapIrradiance": {
					fa = Scene.world == null ? WorldData.getEmptyIrradiance() : Scene.world._irradiance;
					break;
				}
			}

			if (fa != null) {
				Graphics4.setFloats(location, fa);
				return true;
			}
		}
		else if (c.type == "int") {
			let i: Null<i32> = null;
			switch (c.link) {
				case "_envmapNumMipmaps": {
					let w = Scene.world;
					i = w != null ? w.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
					break;
				}
				default:
					return false;
			}

			Graphics4.setInt(location, i != null ? i : 0);
			return true;
		}
		return false;
	}

	static setObjectConstant = (g: Graphics4Raw, object: TBaseObject, location: ConstantLocation, c: TShaderConstant) => {
		if (c.link == null) return;

		let camera = Scene.camera;
		let light = RenderPath.light;

		if (c.type == "mat4") {
			let m: TMat4 = null;
			switch (c.link) {
				case "_worldMatrix": {
					m = object.transform.worldUnpack;
					break;
				}
				case "_inverseWorldMatrix": {
					Mat4.getInverse(Uniforms.helpMat, object.transform.worldUnpack);
					m = Uniforms.helpMat;
					break;
				}
				case "_worldViewProjectionMatrix": {
					Mat4.setFrom(Uniforms.helpMat, object.transform.worldUnpack);
					Mat4.multmat(Uniforms.helpMat, camera.V);
					Mat4.multmat(Uniforms.helpMat, camera.P);
					m = Uniforms.helpMat;
					break;
				}
				case "_worldViewMatrix": {
					Mat4.setFrom(Uniforms.helpMat, object.transform.worldUnpack);
					Mat4.multmat(Uniforms.helpMat, camera.V);
					m = Uniforms.helpMat;
					break;
				}
				case "_prevWorldViewProjectionMatrix": {
					Mat4.setFrom(Uniforms.helpMat, object.ext.prevMatrix);
					Mat4.multmat(Uniforms.helpMat, camera.prevV);
					// helpMat.multmat(camera.prevP);
					Mat4.multmat(Uniforms.helpMat, camera.P);
					m = Uniforms.helpMat;
					break;
				}
				///if arm_particles
				case "_particleData": {
					let mo = object.ext;
					if (mo.particleOwner != null && mo.particleOwner.particleSystems != null) {
						m = ParticleSystem.getData(mo.particleOwner.particleSystems[mo.particleIndex]);
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
			Graphics4.setMatrix(location, m);
		}
		else if (c.type == "mat3") {
			let m: Mat3 = null;
			switch (c.link) {
				case "_normalMatrix": {
					Mat4.getInverse(Uniforms.helpMat, object.transform.world);
					Mat4.transpose3x3(Uniforms.helpMat);
					mat3_setFrom4(Uniforms.helpMat3, Uniforms.helpMat);
					m = Uniforms.helpMat3;
					break;
				}
				case "_viewMatrix3": {
					mat3_setFrom4(Uniforms.helpMat3, camera.V);
					m = Uniforms.helpMat3;
					break;
				}
			}

			if (m == null) return;
			Graphics4.setMatrix3(location, m);
		}
		else if (c.type == "vec4") {
			let v: TVec4 = null;
			Vec4.set(Uniforms.helpVec, 0, 0, 0);

			if (v == null && Uniforms.externalVec4Links != null) {
				for (let fn of Uniforms.externalVec4Links) {
					v = fn(object, Uniforms.currentMat(object), c.link);
					if (v != null) break;
				}
			}

			if (v == null) return;
			Graphics4.setFloat4(location, v.x, v.y, v.z, v.w);
		}
		else if (c.type == "vec3") {
			let v: TVec4 = null;
			Vec4.set(Uniforms.helpVec, 0, 0, 0);
			switch (c.link) {
				case "_dim": { // Model space
					let d = object.transform.dim;
					let s = object.transform.scale;
					Vec4.set(Uniforms.helpVec, (d.x / s.x), (d.y / s.y), (d.z / s.z));
					v = Uniforms.helpVec;
					break;
				}
				case "_halfDim": { // Model space
					let d = object.transform.dim;
					let s = object.transform.scale;
					Vec4.set(Uniforms.helpVec, (d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
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
			Graphics4.setFloat3(location, v.x, v.y, v.z);
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
			Graphics4.setFloat2(location, vx, vy);
		}
		else if (c.type == "float") {
			let f: Null<f32> = null;
			switch (c.link) {
				case "_objectInfoIndex": {
					f = object.uid;
					break;
				}
				case "_objectInfoMaterialIndex": {
					f = Uniforms.currentMat(object)._uid;
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
			Graphics4.setFloat(location, f);
		}
		else if (c.type == "floats") {
			let fa: Float32Array = null;
			switch (c.link) {
				///if arm_skin
				case "_skinBones": {
					if (object.animation != null) {
						fa = object.animation.ext.skinBuffer;
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
			Graphics4.setFloats(location, fa);
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
			Graphics4.setInt(location, i);
		}
	}

	static setMaterialConstants = (g: Graphics4Raw, context: TShaderContext, materialContext: TMaterialContext) => {
		if (materialContext.bind_constants != null) {
			for (let i = 0; i < materialContext.bind_constants.length; ++i) {
				let matc = materialContext.bind_constants[i];
				let pos = -1;
				for (let i = 0; i < context.constants.length; ++i) {
					if (context.constants[i].name == matc.name) {
						pos = i;
						break;
					}
				}
				if (pos == -1) continue;
				let c = context.constants[pos];

				Uniforms.setMaterialConstant(g, context._constants[pos], c, matc);
			}
		}

		if (materialContext._textures != null) {
			for (let i = 0; i < materialContext._textures.length; ++i) {
				let mname = materialContext.bind_textures[i].name;

				for (let j = 0; j < context._textureUnits.length; ++j) {
					let sname = context.texture_units[j].name;
					if (mname == sname) {
						Graphics4.setTexture(context._textureUnits[j], materialContext._textures[i]);
						// After texture sampler have been assigned, set texture parameters
						MaterialContext.setTextureParameters(materialContext, g, i, context, j);
						break;
					}
				}
			}
		}
	}

	static currentMat = (object: TBaseObject): TMaterialData => {
		if (object != null && object.ext != null && object.ext.materials != null) {
			let mo = object.ext;
			return mo.materials[mo.materialIndex];
		}
		return null;
	}

	static setMaterialConstant = (g: Graphics4Raw, location: ConstantLocation, c: TShaderConstant, matc: TBindConstant) => {
		switch (c.type) {
			case "vec4":
				Graphics4.setFloat4(location, matc.vec4[0], matc.vec4[1], matc.vec4[2], matc.vec4[3]);
				break;
			case "vec3":
				Graphics4.setFloat3(location, matc.vec3[0], matc.vec3[1], matc.vec3[2]);
				break;
			case "vec2":
				Graphics4.setFloat2(location, matc.vec2[0], matc.vec2[1]);
				break;
			case "float":
				Graphics4.setFloat(location,  matc.float);
				break;
			case "bool":
				Graphics4.setBool(location, matc.bool);
				break;
			case "int":
				Graphics4.setInt(location, matc.int);
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
