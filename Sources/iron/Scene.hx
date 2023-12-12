package iron;

import haxe.Json;
import iron.System;
import iron.SceneFormat;
using StringTools;

class Scene {

	public static var active: Scene = null;
	public static var global: Object = null;
	static var uidCounter = 0;
	public var uid: Int;
	public var raw: TSceneFormat;
	public var root: Object;
	public var sceneParent: Object;
	public var camera: CameraObject;
	public var world: WorldData;

	public var meshes: Array<MeshObject>;
	public var lights: Array<LightObject>;
	public var cameras: Array<CameraObject>;
	#if arm_audio
	public var speakers: Array<SpeakerObject>;
	#end
	public var empties: Array<Object>;
	public var animations: Array<Animation>;
	#if arm_skin
	public var armatures: Array<Armature>;
	#end
	var groups: Map<String, Array<Object>> = null;

	public var embedded: Map<String, Image>;

	public var ready: Bool; // Async in progress

	public var traitInits: Array<Void->Void> = [];
	public var traitRemoves: Array<Void->Void> = [];

	var initializing: Bool; // Is the scene in its initialization phase?

	public function new() {
		uid = uidCounter++;
		meshes = [];
		lights = [];
		cameras = [];
		#if arm_audio
		speakers = [];
		#end
		empties = [];
		animations = [];
		#if arm_skin
		armatures = [];
		#end
		embedded = new Map();
		root = new Object();
		root.name = "Root";
		traitInits = [];
		traitRemoves = [];
		initializing = true;
		if (global == null) global = new Object();
	}

	public static function create(format: TSceneFormat, done: Object->Void) {
		active = new Scene();
		active.ready = false;
		active.raw = format;

		Data.getWorld(format.name, format.world_ref, function(world: WorldData) {
			active.world = world;

			// Startup scene
			active.addScene(format.name, null, function(sceneObject: Object) {
				if (active.cameras.length == 0) {
					Krom.log('No camera found for scene "' + format.name + '"');
				}

				active.camera = active.getCamera(format.camera_ref);
				active.sceneParent = sceneObject;

				active.ready = true;

				for (f in active.traitInits) f();
				active.traitInits = [];

				active.initializing = false;
				done(sceneObject);
			});
		});
	}

	public function remove() {
		for (f in traitRemoves) f();
		for (o in meshes) o.remove();
		for (o in lights) o.remove();
		for (o in cameras) o.remove();
		#if arm_audio
		for (o in speakers) o.remove();
		#end
		for (o in empties) o.remove();
		groups = null;
		root.remove();
	}

	static var framePassed = true;
	public static function setActive(sceneName: String, done: Object->Void = null) {
		if (!framePassed) return;
		framePassed = false;

		// Defer unloading the world shader until the new world shader is loaded
		// to prevent errors due to a missing world shader inbetween
		var removeWorldShader: String = null;

		if (Scene.active != null) {
			#if (rp_background == "World")
			if (Scene.active.raw.world_ref != null) {
				removeWorldShader = "shader_datas/World_" + Scene.active.raw.world_ref + "/World_" + Scene.active.raw.world_ref;
			}
			#end
			Scene.active.remove();
		}

		Data.getSceneRaw(sceneName, function(format: TSceneFormat) {
			Scene.create(format, function(o: Object) {
				if (done != null) done(o);
				#if arm_voxels // Revoxelize
				RenderPath.active.voxelized = 0;
				#end

				#if (rp_background == "World")
				if (removeWorldShader != null) {
					RenderPath.active.unloadShader(removeWorldShader);
				}
				if (format.world_ref != null) {
					RenderPath.active.loadShader("shader_datas/World_" + format.world_ref + "/World_" + format.world_ref);
				}
				#end
			});
		});
	}

	public function updateFrame() {
		if (!ready) return;
		for (anim in animations) anim.update(Time.delta);
		for (e in empties) if (e != null && e.parent != null) e.transform.update();
	}

	public function renderFrame(g: Graphics4) {
		if (!ready || RenderPath.active == null) return;
		framePassed = true;

		// Render active camera
		camera != null ? camera.renderFrame(g) : RenderPath.active.renderFrame(g);
	}

	// Objects
	public function addObject(parent: Object = null): Object {
		var object = new Object();
		parent != null ? object.setParent(parent) : object.setParent(root);
		return object;
	}

	public function getChildren(?recursive = false): Array<Object> {
		return root.getChildren(recursive);
	}

	public function getChild(name: String): Object {
		return root.getChild(name);
	}

	public function getMesh(name: String): MeshObject {
		for (m in meshes) if (m.name == name) return m;
		return null;
	}

	public function getLight(name: String): LightObject {
		for (l in lights) if (l.name == name) return l;
		return null;
	}

	public function getCamera(name: String): CameraObject {
		for (c in cameras) if (c.name == name) return c;
		return null;
	}

	#if arm_audio
	public function getSpeaker(name: String): SpeakerObject {
		for (s in speakers) if (s.name == name) return s;
		return null;
	}
	#end

	public function getEmpty(name: String): Object {
		for (e in empties) if (e.name == name) return e;
		return null;
	}

	public function addMeshObject(data: MeshData, materials: Array<MaterialData>, parent: Object = null): MeshObject {
		var object = new MeshObject(data, materials);
		parent != null ? object.setParent(parent) : object.setParent(root);
		return object;
	}

	public function addLightObject(data: LightData, parent: Object = null): LightObject {
		var object = new LightObject(data);
		parent != null ? object.setParent(parent) : object.setParent(root);
		return object;
	}

	public function addCameraObject(data: CameraData, parent: Object = null): CameraObject {
		var object = new CameraObject(data);
		parent != null ? object.setParent(parent) : object.setParent(root);
		return object;
	}

	#if arm_audio
	public function addSpeakerObject(data: TSpeakerData, parent: Object = null): SpeakerObject {
		var object = new SpeakerObject(data);
		parent != null ? object.setParent(parent) : object.setParent(root);
		return object;
	}
	#end

	public function addScene(sceneName: String, parent: Object, done: Object->Void) {
		if (parent == null) {
			parent = addObject();
			parent.name = sceneName;
		}
		Data.getSceneRaw(sceneName, function(format: TSceneFormat) {
			loadEmbeddedData(format.embedded_datas, function() { // Additional scene assets
				var objectsTraversed = 0;

				var objectsCount = getObjectsCount(format.objects);
				function traverseObjects(parent: Object, objects: Array<TObj>, parentObject: TObj, done: Void->Void) {
					if (objects == null) return;
					for (i in 0...objects.length) {
						var o = objects[i];
						if (o.spawn != null && o.spawn == false) {
							if (++objectsTraversed == objectsCount) done();
							continue; // Do not auto-create this object
						}

						createObject(o, format, parent, parentObject, function(object: Object) {
							traverseObjects(object, o.children, o, done);
							if (++objectsTraversed == objectsCount) done();
						});
					}
				}

				if (format.objects == null || format.objects.length == 0) {
					done(parent);
				}
				else {
					traverseObjects(parent, format.objects, null, function() { // Scene objects
						done(parent);
					});
				}
			});
		});
	}

	function getObjectsCount(objects: Array<TObj>, discardNoSpawn = true): Int {
		if (objects == null) return 0;
		var result = objects.length;
		for (o in objects) {
			if (discardNoSpawn && o.spawn != null && o.spawn == false) continue; // Do not count children of non-spawned objects
			if (o.children != null) result += getObjectsCount(o.children);
		}
		return result;
	}

	public function spawnObject(name: String, parent: Null<Object>, done: Null<Object->Void>, spawnChildren = true, srcRaw: Null<TSceneFormat> = null) {
		if (srcRaw == null) srcRaw = raw;
		var objectsTraversed = 0;
		var obj = getRawObjectByName(srcRaw, name);
		var objectsCount = spawnChildren ? getObjectsCount([obj], false) : 1;
		var rootId = -1;
		function spawnObjectTree(obj: TObj, parent: Object, parentObject: TObj, done: Object->Void) {
			createObject(obj, srcRaw, parent, parentObject, function(object: Object) {
				if (rootId == -1) {
					rootId = object.uid;
				}
				if (spawnChildren && obj.children != null) {
					for (child in obj.children) spawnObjectTree(child, object, obj, done);
				}
				if (++objectsTraversed == objectsCount && done != null) {
					// Retrieve the originally spawned object from the current
					// child object to ensure done() is called with the right
					// object
					while (object.uid != rootId) {
						object = object.parent;
					}
					done(object);
				}
			});
		}
		spawnObjectTree(obj, parent, null, done);
	}

	public function parseObject(sceneName: String, objectName: String, parent: Object, done: Object->Void) {
		Data.getSceneRaw(sceneName, function(format: TSceneFormat) {
			var o: TObj = getRawObjectByName(format, objectName);
			if (o == null) done(null);
			createObject(o, format, parent, null, done);
		});
	}

	public static function getRawObjectByName(format: TSceneFormat, name: String): TObj {
		return traverseObjs(format.objects, name);
	}

	static function traverseObjs(children: Array<TObj>, name: String): TObj {
		for (o in children) {
			if (o.name == name) return o;
			if (o.children != null) {
				var res = traverseObjs(o.children, name);
				if (res != null) return res;
			}
		}
		return null;
	}

	public function createObject(o: TObj, format: TSceneFormat, parent: Object, parentObject: TObj, done: Object->Void) {
		var sceneName = format.name;

		if (o.type == "camera_object") {
			Data.getCamera(sceneName, o.data_ref, function(b: CameraData) {
				var object = addCameraObject(b, parent);
				returnObject(object, o, done);
			});
		}
		else if (o.type == "light_object") {
			Data.getLight(sceneName, o.data_ref, function(b: LightData) {
				var object = addLightObject(b, parent);
				returnObject(object, o, done);
			});
		}
		else if (o.type == "mesh_object") {
			if (o.material_refs == null || o.material_refs.length == 0) {
				createMeshObject(o, format, parent, parentObject, null, done);
			}
			else {
				// Materials
				var materials: Array<MaterialData> = [];
				var materialsLoaded = 0;
				for (i in 0...o.material_refs.length) {
					var ref = o.material_refs[i];
					Data.getMaterial(sceneName, ref, function(mat: MaterialData) {
						materials[i] = mat;
						materialsLoaded++;
						if (materialsLoaded == o.material_refs.length) {
							createMeshObject(o, format, parent, parentObject, materials, done);
						}
					});
				}
			}
		}
		#if arm_audio
		else if (o.type == "speaker_object") {
			var object = addSpeakerObject(Data.getSpeakerRawByName(format.speaker_datas, o.data_ref), parent);
			returnObject(object, o, done);
		}
		#end
		else if (o.type == "object") {
			var object = addObject(parent);
			returnObject(object, o, function(ro: Object) {
				done(ro);
			});
		}
		else done(null);
	}

	function getChildObjectsRaw(rawObj: TObj, ?recursive:Bool = true): Array<TObj> {
		var children = rawObj.children;
		if (children == null) return new Array<TObj>();
		children = children.copy();

		if (recursive) {
			for (child in rawObj.children) {
				var childRefs = getChildObjectsRaw(child);
				children = children.concat(childRefs);
			}
		}
		return children;
	}

	function createMeshObject(o: TObj, format: TSceneFormat, parent: Object, parentObject: TObj, materials: Array<MaterialData>, done: Object->Void) {
		// Mesh reference
		var ref = o.data_ref.split("/");
		var object_file = "";
		var data_ref = "";
		var sceneName = format.name;
		if (ref.length == 2) { // File reference
			object_file = ref[0];
			data_ref = ref[1];
		}
		else { // Local mesh data
			object_file = sceneName;
			data_ref = o.data_ref;
		}

		// Bone objects are stored in armature parent
		#if arm_skin
		if (parentObject != null && parentObject.bone_actions != null) {
			var bactions: Array<TSceneFormat> = [];
			for (ref in parentObject.bone_actions) {
				Data.getSceneRaw(ref, function(action: TSceneFormat) {
					bactions.push(action);
					if (bactions.length == parentObject.bone_actions.length) {
						var armature: Armature = null;
						// Check if armature exists
						for (a in armatures) {
							if (a.uid == parent.uid) {
								armature = a;
								break;
							}
						}
						// Create new one
						if (armature == null) {
							// Unique name if armature was already instantiated for different object
							for (a in armatures) {
								if (a.name == parent.name) {
									parent.name += "." + parent.uid;
									break;
								}
							}
							armature = new Armature(parent.uid, parent.name, bactions);
							armatures.push(armature);
						}
						returnMeshObject(
							object_file, data_ref, sceneName, armature, materials, parent, parentObject, o, done);
					}
				});
			}
		}
		else { #end // arm_skin
			returnMeshObject(
				object_file, data_ref, sceneName, null, materials, parent, parentObject, o, done);
		#if arm_skin
		}
		#end
	}

	public function returnMeshObject(object_file: String, data_ref: String, sceneName: String, armature: #if arm_skin Armature #else Null<Int> #end, materials: Array<MaterialData>, parent: Object, parentObject: TObj, o: TObj, done: Object->Void) {
		Data.getMesh(object_file, data_ref, function(mesh: MeshData) {
			#if arm_skin
			if (mesh.isSkinned) {
				var g = mesh.geom;
				armature != null ? g.addArmature(armature) : g.addAction(mesh.format.objects, "none");
			}
			#end
			var object = addMeshObject(mesh, materials, parent);

			// Attach particle systems
			#if arm_particles
			if (o.particle_refs != null) {
				for (ref in o.particle_refs) cast(object, MeshObject).setupParticleSystem(sceneName, ref);
			}
			#end
			returnObject(object, o, done);
		});
	}

	function returnObject(object: Object, o: TObj, done: Object->Void) {
		// Load object actions
		if (object != null && o.object_actions != null) {
			var oactions: Array<TSceneFormat> = [];
			while (oactions.length < o.object_actions.length) oactions.push(null);
			var actionsLoaded = 0;
			for (i in 0...o.object_actions.length) {
				var ref = o.object_actions[i];
				if (ref == "null") { // No startup action set
					actionsLoaded++;
					continue;
				}
				Data.getSceneRaw(ref, function(action: TSceneFormat) {
					oactions[i] = action;
					actionsLoaded++;
					if (actionsLoaded == o.object_actions.length) {
						returnObjectLoaded(object, o, oactions, done);
					}
				});
			}
		}
		else returnObjectLoaded(object, o, null, done);
	}

	function returnObjectLoaded(object: Object, o: TObj, oactions: Array<TSceneFormat>, done: Object->Void) {
		if (object != null) {
			object.raw = o;
			object.name = o.name;
			if (o.visible != null) object.visible = o.visible;
			if (o.visible_mesh != null) object.visibleMesh = o.visible_mesh;
			generateTransform(o, object.transform);
			object.setupAnimation(oactions);
		}
		done(object);
	}

	static function generateTransform(object: TObj, transform: Transform) {
		transform.world = object.transform != null ? Mat4.fromFloat32Array(object.transform.values) : Mat4.identity();
		transform.world.decompose(transform.loc, transform.rot, transform.scale);
		// Whether to apply parent matrix
		if (object.local_only != null) transform.localOnly = object.local_only;
		if (transform.object.parent != null) transform.update();
	}

 	static function parseArg(str: String): Dynamic {
		if (str == "true") return true;
		else if (str == "false") return false;
		else if (str == "null") return null;
		else if (str.charAt(0) == "'") return str.replace("'", "");
		else if (str.charAt(0) == '"') return str.replace('"', "");
		else if (str.charAt(0) == "[") { // Array
			// Remove [] and recursively parse into array, then append into parent
			str = str.replace("[", "");
			str = str.replace("]", "");
			str = str.replace(" ", "");
			var ar: Dynamic = [];
			var vals = str.split(",");
			for (v in vals) ar.push(parseArg(v));
			return ar;
		}
		else if (str.charAt(0) == '{') { // Typedef or anonymous structure
			return Json.parse(str);
		}
		else {
			var f = Std.parseFloat(str);
			var i = Std.parseInt(str);
			return f == i ? i : f;
		}
	}

	static function createTraitClassInstance(traitName: String, args: Array<Dynamic>): Dynamic {
		var cname = Type.resolveClass(traitName);
		if (cname == null) return null;
		return Type.createInstance(cname, args);
	}

	function loadEmbeddedData(datas: Array<String>, done: Void->Void) {
		if (datas == null) {
			done();
			return;
		}
		var loaded = 0;
		for (file in datas) {
			embedData(file, function() {
				loaded++;
				if (loaded == datas.length) done();
			});
		}
	}

	public function embedData(file: String, done: Void->Void) {
		if (file.endsWith(".raw")) {
			Data.getBlob(file, function(b: js.lib.ArrayBuffer) {
				// Raw 3D texture bytes
				var w = Std.int(Math.pow(b.byteLength, 1 / 3)) + 1;
				var image = Image.fromBytes3D(b, w, w, w, TextureFormat.R8);
				embedded.set(file, image);
				done();
			});
		}
		else {
			Data.getImage(file, function(image: Image) {
				embedded.set(file, image);
				done();
			});
		}
	}

	// Hooks
	public function notifyOnInit(f: Void->Void) {
		if (ready) f(); // Scene already running
		else traitInits.push(f);
	}

	public function removeInit(f: Void->Void) {
		traitInits.remove(f);
	}

	public function notifyOnRemove(f: Void->Void) {
		traitRemoves.push(f);
	}
}
