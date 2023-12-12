package iron;

import js.lib.Float32Array;
import iron.System;
import iron.SceneFormat;
import iron.ShaderData;
import iron.MaterialData;
import iron.RenderPath;
using StringTools;

// Structure for setting shader uniforms
class Uniforms {

	public static var helpMat = Mat4.identity();
	public static var helpMat2 = Mat4.identity();
	public static var helpMat3 = Mat3.identity();
	public static var helpVec = new Vec4();
	public static var helpVec2 = new Vec4();
	public static var helpQuat = new Quat(); // Keep at identity

	public static var externalTextureLinks: Array<Object->MaterialData->String->Image> = null;
	public static var externalMat4Links: Array<Object->MaterialData->String->Mat4> = null;
	public static var externalVec4Links: Array<Object->MaterialData->String->Vec4> = null;
	public static var externalVec3Links: Array<Object->MaterialData->String->Vec4> = null;
	public static var externalVec2Links: Array<Object->MaterialData->String->Vec4> = null;
	public static var externalFloatLinks: Array<Object->MaterialData->String->Null<Float>> = null;
	public static var externalFloatsLinks: Array<Object->MaterialData->String->Float32Array> = null;
	public static var externalIntLinks: Array<Object->MaterialData->String->Null<Int>> = null;
	public static var posUnpack: Null<Float> = null;
	public static var texUnpack: Null<Float> = null;

	public static function setContextConstants(g: Graphics4, context: ShaderContext, bindParams: Array<String>) {
		if (context.raw.constants != null) {
			for (i in 0...context.raw.constants.length) {
				var c = context.raw.constants[i];
				setContextConstant(g, context.constants[i], c);
			}
		}

		// Texture context constants
		if (bindParams != null) { // Bind targets
			for (i in 0...Std.int(bindParams.length / 2)) {
				var pos = i * 2; // bind params = [texture, samplerID]
				var rtID = bindParams[pos];
				var samplerID = bindParams[pos + 1];
				var attachDepth = false; // Attach texture depth if '_' is prepended
				var char = rtID.charAt(0);
				if (char == "_") {
					attachDepth = true;
					rtID = rtID.substr(1);
				}
				var rt = attachDepth ? RenderPath.active.depthToRenderTarget.get(rtID) : RenderPath.active.renderTargets.get(rtID);
				bindRenderTarget(g, rt, context, samplerID, attachDepth);
			}
		}

		// Texture links
		if (context.raw.texture_units != null) {
			for (j in 0...context.raw.texture_units.length) {
				var tulink = context.raw.texture_units[j].link;
				if (tulink == null) continue;

				if (tulink.charAt(0) == "$") { // Link to embedded data
					g.setTexture(context.textureUnits[j], Scene.active.embedded.get(tulink.substr(1)));
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
							var w = Scene.active.world;
							if (w != null) {
								g.setTexture(context.textureUnits[j], w.probe.radiance);
								g.setTextureParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.LinearMipFilter);
							}
						}
						case "_envmap": {
							var w = Scene.active.world;
							if (w != null) {
								g.setTexture(context.textureUnits[j], w.envmap);
								g.setTextureParameters(context.textureUnits[j], TextureAddressing.Repeat, TextureAddressing.Repeat, TextureFilter.LinearFilter, TextureFilter.LinearFilter, MipMapFilter.NoMipFilter);
							}
						}
					}
				}
			}
		}
	}

	public static function setObjectConstants(g: Graphics4, context: ShaderContext, object: Object) {
		if (context.raw.constants != null) {
			for (i in 0...context.raw.constants.length) {
				var c = context.raw.constants[i];
				setObjectConstant(g, object, context.constants[i], c);
			}
		}

		// Texture object constants
		// External
		if (externalTextureLinks != null) {
			if (context.raw.texture_units != null) {
				for (j in 0...context.raw.texture_units.length) {
					var tu = context.raw.texture_units[j];
					if (tu.link == null) continue;
					var tuAddrU = getTextureAddressing(tu.addressing_u);
					var tuAddrV = getTextureAddressing(tu.addressing_v);
					var tuFilterMin = getTextureFilter(tu.filter_min);
					var tuFilterMag = getTextureFilter(tu.filter_mag);
					var tuMipMapFilter = getMipMapFilter(tu.mipmap_filter);

					for (f in externalTextureLinks) {
						var image = f(object, currentMat(object), tu.link);
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

	static function bindRenderTarget(g: Graphics4, rt: RenderTarget, context: ShaderContext, samplerID: String, attachDepth: Bool) {
		if (rt != null) {
			var tus = context.raw.texture_units;

			for (j in 0...tus.length) { // Set texture
				if (samplerID == tus[j].name) {
					var isImage = tus[j].is_image != null && tus[j].is_image;
					var paramsSet = false;

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
						var oc = context.overrideContext;
						var allowParams = oc == null || oc.shared_sampler == null || oc.shared_sampler == samplerID;
						if (allowParams) {
							var addressing = (oc != null && oc.addressing == "repeat") ? TextureAddressing.Repeat : TextureAddressing.Clamp;
							var filter = (oc != null && oc.filter == "point") ? TextureFilter.PointFilter : TextureFilter.LinearFilter;
							g.setTextureParameters(context.textureUnits[j], addressing, addressing, filter, filter, MipMapFilter.NoMipFilter);
						}
						paramsSet = true;
					}
				}
			}
		}
	}

	static function setContextConstant(g: Graphics4, location: ConstantLocation, c: TShaderConstant): Bool {
		if (c.link == null) return true;

		var camera = Scene.active.camera;
		var light = RenderPath.active.light;

		if (c.type == "mat4") {
			var m: Mat4 = null;
			switch (c.link) {
				case "_viewMatrix": {
					m = camera.V;
				}
				case "_projectionMatrix": {
					m = camera.P;
				}
				case "_inverseProjectionMatrix": {
					helpMat.getInverse(camera.P);
					m = helpMat;
				}
				case "_viewProjectionMatrix": {
					m = camera.VP;
				}
				case "_inverseViewProjectionMatrix": {
					helpMat.setFrom(camera.V);
					helpMat.multmat(camera.P);
					helpMat.getInverse(helpMat);
					m = helpMat;
				}
				case "_skydomeMatrix": {
					var tr = camera.transform;
					helpVec.set(tr.worldx(), tr.worldy(), tr.worldz() - 3.5); // Sky
					var bounds = camera.data.raw.far_plane * 0.95;
					helpVec2.set(bounds, bounds, bounds);
					helpMat.compose(helpVec, helpQuat, helpVec2);
					helpMat.multmat(camera.V);
					helpMat.multmat(camera.P);
					m = helpMat;
				}
				default: // Unknown uniform
					return false;
			}

			g.setMatrix(location, m != null ? m : Mat4.identity());
			return true;
		}
		else if (c.type == "vec4") {
			var v: Vec4 = null;
			helpVec.set(0, 0, 0, 0);
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
			var v: Vec4 = null;
			helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_lightDirection": {
					if (light != null) {
						helpVec = light.look().normalize();
						v = helpVec;
					}
				}
				case "_pointPosition": {
					var point = RenderPath.active.point;
					if (point != null) {
						helpVec.set(point.transform.worldx(), point.transform.worldy(), point.transform.worldz());
						v = helpVec;
					}
				}
				case "_pointColor": {
					var point = RenderPath.active.point;
					if (point != null) {
						var str = point.visible ? point.data.raw.strength : 0.0;
						helpVec.set(point.data.raw.color[0] * str, point.data.raw.color[1] * str, point.data.raw.color[2] * str);
						v = helpVec;
					}
				}
				case "_lightArea0": {
					if (light != null && light.data.raw.size != null) {
						var f2: Float = 0.5;
						var sx: Float = light.data.raw.size * f2;
						var sy: Float = light.data.raw.size_y * f2;
						helpVec.set(-sx, sy, 0.0);
						helpVec.applymat(light.transform.world);
						v = helpVec;
					}
				}
				case "_lightArea1": {
					if (light != null && light.data.raw.size != null) {
						var f2: Float = 0.5;
						var sx: Float = light.data.raw.size * f2;
						var sy: Float = light.data.raw.size_y * f2;
						helpVec.set(sx, sy, 0.0);
						helpVec.applymat(light.transform.world);
						v = helpVec;
					}
				}
				case "_lightArea2": {
					if (light != null && light.data.raw.size != null) {
						var f2: Float = 0.5;
						var sx: Float = light.data.raw.size * f2;
						var sy: Float = light.data.raw.size_y * f2;
						helpVec.set(sx, -sy, 0.0);
						helpVec.applymat(light.transform.world);
						v = helpVec;
					}
				}
				case "_lightArea3": {
					if (light != null && light.data.raw.size != null) {
						var f2: Float = 0.5;
						var sx: Float = light.data.raw.size * f2;
						var sy: Float = light.data.raw.size_y * f2;
						helpVec.set(-sx, -sy, 0.0);
						helpVec.applymat(light.transform.world);
						v = helpVec;
					}
				}
				case "_cameraPosition": {
					helpVec.set(camera.transform.worldx(), camera.transform.worldy(), camera.transform.worldz());
					v = helpVec;
				}
				case "_cameraLook": {
					helpVec = camera.lookWorld().normalize();
					v = helpVec;
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
			var v: Vec4 = null;
			helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_vec2x": {
					v = helpVec;
					v.x = 1.0;
					v.y = 0.0;
				}
				case "_vec2xInv": {
					v = helpVec;
					v.x = 1.0 / RenderPath.active.currentW;
					v.y = 0.0;
				}
				case "_vec2x2": {
					v = helpVec;
					v.x = 2.0;
					v.y = 0.0;
				}
				case "_vec2x2Inv": {
					v = helpVec;
					v.x = 2.0 / RenderPath.active.currentW;
					v.y = 0.0;
				}
				case "_vec2y": {
					v = helpVec;
					v.x = 0.0;
					v.y = 1.0;
				}
				case "_vec2yInv": {
					v = helpVec;
					v.x = 0.0;
					v.y = 1.0 / RenderPath.active.currentH;
				}
				case "_vec2y2": {
					v = helpVec;
					v.x = 0.0;
					v.y = 2.0;
				}
				case "_vec2y2Inv": {
					v = helpVec;
					v.x = 0.0;
					v.y = 2.0 / RenderPath.active.currentH;
				}
				case "_vec2y3": {
					v = helpVec;
					v.x = 0.0;
					v.y = 3.0;
				}
				case "_vec2y3Inv": {
					v = helpVec;
					v.x = 0.0;
					v.y = 3.0 / RenderPath.active.currentH;
				}
				case "_screenSize": {
					v = helpVec;
					v.x = RenderPath.active.currentW;
					v.y = RenderPath.active.currentH;
				}
				case "_screenSizeInv": {
					v = helpVec;
					v.x = 1.0 / RenderPath.active.currentW;
					v.y = 1.0 / RenderPath.active.currentH;
				}
				case "_cameraPlaneProj": {
					var near = camera.data.raw.near_plane;
					var far = camera.data.raw.far_plane;
					v = helpVec;
					v.x = far / (far - near);
					v.y = (-far * near) / (far - near);
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
			var f: Null<Float> = null;
			switch (c.link) {
				case "_time": {
					f = Time.time();
				}
				case "_aspectRatioWindowF": {
					f = App.w() / App.h();
				}
				default:
					return false;
			}

			g.setFloat(location, f != null ? f : 0);
			return true;
		}
		else if (c.type == "floats") {
			var fa: Float32Array = null;
			switch (c.link) {
				case "_envmapIrradiance": {
					fa = Scene.active.world == null ? WorldData.getEmptyIrradiance() : Scene.active.world.probe.irradiance;
				}
			}

			if (fa != null) {
				g.setFloats(location, fa);
				return true;
			}
		}
		else if (c.type == "int") {
			var i: Null<Int> = null;
			switch (c.link) {
				case "_envmapNumMipmaps": {
					var w = Scene.active.world;
					i = w != null ? w.probe.raw.radiance_mipmaps + 1 - 2 : 1; // Include basecolor and exclude 2 scaled mips
				}
				default:
					return false;
			}

			g.setInt(location, i != null ? i : 0);
			return true;
		}
		return false;
	}

	static function setObjectConstant(g: Graphics4, object: Object, location: ConstantLocation, c: TShaderConstant) {
		if (c.link == null) return;

		var camera = Scene.active.camera;
		var light = RenderPath.active.light;

		if (c.type == "mat4") {
			var m: Mat4 = null;
			switch (c.link) {
				case "_worldMatrix": {
					m = object.transform.worldUnpack;
				}
				case "_inverseWorldMatrix": {
					helpMat.getInverse(object.transform.worldUnpack);
					m = helpMat;
				}
				case "_worldViewProjectionMatrix": {
					helpMat.setFrom(object.transform.worldUnpack);
					helpMat.multmat(camera.V);
					helpMat.multmat(camera.P);
					m = helpMat;
				}
				case "_worldViewMatrix": {
					helpMat.setFrom(object.transform.worldUnpack);
					helpMat.multmat(camera.V);
					m = helpMat;
				}
				case "_prevWorldViewProjectionMatrix": {
					helpMat.setFrom(cast(object, MeshObject).prevMatrix);
					helpMat.multmat(camera.prevV);
					// helpMat.multmat(camera.prevP);
					helpMat.multmat(camera.P);
					m = helpMat;
				}
				#if arm_particles
				case "_particleData": {
					var mo = cast(object, MeshObject);
					if (mo.particleOwner != null && mo.particleOwner.particleSystems != null) {
						m = mo.particleOwner.particleSystems[mo.particleIndex].getData();
					}
				}
				#end
			}

			if (m == null && externalMat4Links != null) {
				for (fn in externalMat4Links) {
					m = fn(object, currentMat(object), c.link);
					if (m != null) break;
				}
			}

			if (m == null) return;
			g.setMatrix(location, m);
		}
		else if (c.type == "mat3") {
			var m: Mat3 = null;
			switch (c.link) {
				case "_normalMatrix": {
					helpMat.getInverse(object.transform.world);
					helpMat.transpose3x3();
					helpMat3.setFrom4(helpMat);
					m = helpMat3;
				}
				case "_viewMatrix3": {
					helpMat3.setFrom4(camera.V);
					m = helpMat3;
				}
			}

			if (m == null) return;
			g.setMatrix3(location, m);
		}
		else if (c.type == "vec4") {
			var v: Vec4 = null;
			helpVec.set(0, 0, 0);

			if (v == null && externalVec4Links != null) {
				for (fn in externalVec4Links) {
					v = fn(object, currentMat(object), c.link);
					if (v != null) break;
				}
			}

			if (v == null) return;
			g.setFloat4(location, v.x, v.y, v.z, v.w);
		}
		else if (c.type == "vec3") {
			var v: Vec4 = null;
			helpVec.set(0, 0, 0);
			switch (c.link) {
				case "_dim": { // Model space
					var d = object.transform.dim;
					var s = object.transform.scale;
					helpVec.set((d.x / s.x), (d.y / s.y), (d.z / s.z));
					v = helpVec;
				}
				case "_halfDim": { // Model space
					var d = object.transform.dim;
					var s = object.transform.scale;
					helpVec.set((d.x / s.x) / 2, (d.y / s.y) / 2, (d.z / s.z) / 2);
					v = helpVec;
				}
			}

			if (v == null && externalVec3Links != null) {
				for (f in externalVec3Links) {
					v = f(object, currentMat(object), c.link);
					if (v != null) break;
				}
			}

			if (v == null) return;
			g.setFloat3(location, v.x, v.y, v.z);
		}
		else if (c.type == "vec2") {
			var vx: Null<Float> = null;
			var vy: Null<Float> = null;

			if (vx == null && externalVec2Links != null) {
				for (fn in externalVec2Links) {
					var v = fn(object, currentMat(object), c.link);
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
			var f: Null<Float> = null;
			switch (c.link) {
				case "_objectInfoIndex": {
					f = object.uid;
				}
				case "_objectInfoMaterialIndex": {
					f = currentMat(object).uid;
				}
				case "_objectInfoRandom": {
					f = object.urandom;
				}
				case "_posUnpack": {
					f = posUnpack != null ? posUnpack : 1.0;
				}
				case "_texUnpack": {
					f = texUnpack != null ? texUnpack : 1.0;
				}
			}

			if (f == null && externalFloatLinks != null) {
				for (fn in externalFloatLinks) {
					var res = fn(object, currentMat(object), c.link);
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
			var fa: Float32Array = null;
			switch (c.link) {
				#if arm_skin
				case "_skinBones": {
					if (object.animation != null) {
						fa = cast(object.animation, BoneAnimation).skinBuffer;
					}
				}
				#end
			}

			if (fa == null && externalFloatsLinks != null) {
				for (fn in externalFloatsLinks) {
					fa = fn(object, currentMat(object), c.link);
					if (fa != null) break;
				}
			}

			if (fa == null) return;
			g.setFloats(location, fa);
		}
		else if (c.type == "int") {
			var i: Null<Int> = null;
			switch (c.link) {
				case "_uid": {
					i = object.uid;
				}
			}

			if (i == null && externalIntLinks != null) {
				for (fn in externalIntLinks) {
					var res = fn(object, currentMat(object), c.link);
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

	public static function setMaterialConstants(g: Graphics4, context: ShaderContext, materialContext: MaterialContext) {
		if (materialContext.raw.bind_constants != null) {
			for (i in 0...materialContext.raw.bind_constants.length) {
				var matc = materialContext.raw.bind_constants[i];
				var pos = -1;
				for (i in 0...context.raw.constants.length) {
					if (context.raw.constants[i].name == matc.name) {
						pos = i;
						break;
					}
				}
				if (pos == -1) continue;
				var c = context.raw.constants[pos];

				setMaterialConstant(g, context.constants[pos], c, matc);
			}
		}

		if (materialContext.textures != null) {
			for (i in 0...materialContext.textures.length) {
				var mname = materialContext.raw.bind_textures[i].name;

				for (j in 0...context.textureUnits.length) {
					var sname = context.raw.texture_units[j].name;
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

	static function currentMat(object: Object): MaterialData {
		if (object != null && Std.isOfType(object, MeshObject)) {
			var mo = cast(object, MeshObject);
			return mo.materials[mo.materialIndex];
		}
		return null;
	}

	static function setMaterialConstant(g: Graphics4, location: ConstantLocation, c: TShaderConstant, matc: TBindConstant) {
		switch (c.type) {
			case "vec4": g.setFloat4(location, matc.vec4[0], matc.vec4[1], matc.vec4[2], matc.vec4[3]);
			case "vec3": g.setFloat3(location, matc.vec3[0], matc.vec3[1], matc.vec3[2]);
			case "vec2": g.setFloat2(location, matc.vec2[0], matc.vec2[1]);
			case "float": g.setFloat(location,  matc.float);
			case "bool": g.setBool(location, matc.bool);
			case "int": g.setInt(location, matc.int);
		}
	}

	static inline function getTextureAddressing(s: String): TextureAddressing {
		return switch (s) {
			case "clamp": TextureAddressing.Clamp;
			case "mirror": TextureAddressing.Mirror;
			default: TextureAddressing.Repeat;
		}
	}

	static inline function getTextureFilter(s: String): TextureFilter {
		return switch (s) {
			case "anisotropic": TextureFilter.AnisotropicFilter;
			case "point": TextureFilter.PointFilter;
			default: TextureFilter.LinearFilter;
		}
	}

	static inline function getMipMapFilter(s: String): MipMapFilter {
		return switch (s) {
			case "linear": MipMapFilter.LinearMipFilter;
			case "point": MipMapFilter.PointMipFilter;
			default: MipMapFilter.NoMipFilter;
		}
	}
}
