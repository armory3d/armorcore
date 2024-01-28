
class Scene {

	static active: Scene = null;
	static global: BaseObject = null;
	static uidCounter = 0;
	uid: i32;
	raw: TSceneFormat;
	root: BaseObject;
	sceneParent: BaseObject;
	camera: CameraObject;
	world: WorldData;

	meshes: MeshObject[];
	lights: LightObject[];
	cameras: CameraObject[];
	///if arm_audio
	speakers: SpeakerObject[];
	///end
	empties: BaseObject[];
	animations: Animation[];
	///if arm_skin
	armatures: Armature[];
	///end
	groups: Map<string, BaseObject[]> = null;

	embedded: Map<string, Image>;

	ready: bool; // Async in progress

	traitInits: (()=>void)[] = [];
	traitRemoves: (()=>void)[] = [];

	initializing: bool; // Is the scene in its initialization phase?

	constructor() {
		this.uid = Scene.uidCounter++;
		this.meshes = [];
		this.lights = [];
		this.cameras = [];
		///if arm_audio
		this.speakers = [];
		///end
		this.empties = [];
		this.animations = [];
		///if arm_skin
		this.armatures = [];
		///end
		this.embedded = new Map();
		this.root = new BaseObject();
		this.root.name = "Root";
		this.traitInits = [];
		this.traitRemoves = [];
		this.initializing = true;
		if (Scene.global == null) Scene.global = new BaseObject();
	}

	static create = (format: TSceneFormat, done: (o: BaseObject)=>void) => {
		Scene.active = new Scene();
		Scene.active.ready = false;
		Scene.active.raw = format;

		Data.getWorld(format.name, format.world_ref, (world: WorldData) => {
			Scene.active.world = world;

			// Startup scene
			Scene.active.addScene(format.name, null, (sceneObject: BaseObject) => {
				if (Scene.active.cameras.length == 0) {
					Krom.log('No camera found for scene "' + format.name + '"');
				}

				Scene.active.camera = Scene.active.getCamera(format.camera_ref);
				Scene.active.sceneParent = sceneObject;

				Scene.active.ready = true;

				for (let f of Scene.active.traitInits) f();
				Scene.active.traitInits = [];

				Scene.active.initializing = false;
				done(sceneObject);
			});
		});
	}

	remove = () => {
		for (let f of this.traitRemoves) f();
		for (let o of this.meshes) o.remove();
		for (let o of this.lights) o.remove();
		for (let o of this.cameras) o.remove();
		///if arm_audio
		for (let o of this.speakers) o.remove();
		///end
		for (let o of this.empties) o.remove();
		this.groups = null;
		this.root.remove();
	}

	static framePassed = true;

	static setActive = (sceneName: string, done: (o: BaseObject)=>void = null) => {

		if (!Scene.framePassed) return;
		Scene.framePassed = false;

		// Defer unloading the world shader until the new world shader is loaded
		// to prevent errors due to a missing world shader inbetween
		let removeWorldShader: string = null;

		if (Scene.active != null) {
			///if (rp_background == "World")
			if (Scene.active.raw.world_ref != null) {
				removeWorldShader = "shader_datas/World_" + Scene.active.raw.world_ref + "/World_" + Scene.active.raw.world_ref;
			}
			///end
			Scene.active.remove();
		}

		Data.getSceneRaw(sceneName, (format: TSceneFormat) => {
			Scene.create(format, (o: BaseObject) => {
				if (done != null) done(o);
				///if arm_voxels // Revoxelize
				RenderPath.active.voxelized = 0;
				///end

				///if (rp_background == "World")
				if (removeWorldShader != null) {
					RenderPath.active.unloadShader(removeWorldShader);
				}
				if (format.world_ref != null) {
					RenderPath.active.loadShader("shader_datas/World_" + format.world_ref + "/World_" + format.world_ref);
				}
				///end
			});
		});
	}

	updateFrame = () => {
		if (!this.ready) return;
		for (let anim of this.animations) anim.update(Time.delta);
		for (let e of this.empties) if (e != null && e.parent != null) e.transform.update();
	}

	renderFrame = (g: Graphics4) => {
		if (!this.ready || RenderPath.active == null) return;
		Scene.framePassed = true;

		// Render active camera
		this.camera != null ? this.camera.renderFrame(g) : RenderPath.active.renderFrame(g);
	}

	// Objects
	addObject = (parent: BaseObject = null): BaseObject => {
		let object = new BaseObject();
		parent != null ? object.setParent(parent) : object.setParent(this.root);
		return object;
	}

	getChildren = (recursive = false): BaseObject[] => {
		return this.root.getChildren(recursive);
	}

	getChild = (name: string): BaseObject => {
		return this.root.getChild(name);
	}

	getMesh = (name: string): MeshObject => {
		for (let m of this.meshes) if (m.name == name) return m;
		return null;
	}

	getLight = (name: string): LightObject => {
		for (let l of this.lights) if (l.name == name) return l;
		return null;
	}

	getCamera = (name: string): CameraObject => {
		for (let c of this.cameras) if (c.name == name) return c;
		return null;
	}

	///if arm_audio
	getSpeaker = (name: string): SpeakerObject => {
		for (let s of this.speakers) if (s.name == name) return s;
		return null;
	}
	///end

	getEmpty = (name: string): BaseObject => {
		for (let e of this.empties) if (e.name == name) return e;
		return null;
	}

	addMeshObject = (data: MeshData, materials: MaterialData[], parent: BaseObject = null): MeshObject => {
		let object = new MeshObject(data, materials);
		parent != null ? object.setParent(parent) : object.setParent(this.root);
		return object;
	}

	addLightObject = (data: LightData, parent: BaseObject = null): LightObject => {
		let object = new LightObject(data);
		parent != null ? object.setParent(parent) : object.setParent(this.root);
		return object;
	}

	addCameraObject = (data: CameraData, parent: BaseObject = null): CameraObject => {
		let object = new CameraObject(data);
		parent != null ? object.setParent(parent) : object.setParent(this.root);
		return object;
	}

	///if arm_audio
	addSpeakerObject = (data: TSpeakerData, parent: BaseObject = null): SpeakerObject => {
		let object = new SpeakerObject(data);
		parent != null ? object.setParent(parent) : object.setParent(this.root);
		return object;
	}
	///end

	addScene = (sceneName: string, parent: BaseObject, done: (o: BaseObject)=>void) => {
		if (parent == null) {
			parent = this.addObject();
			parent.name = sceneName;
		}
		Data.getSceneRaw(sceneName, (format: TSceneFormat) => {
			this.loadEmbeddedData(format.embedded_datas, () => { // Additional scene assets
				let objectsTraversed = 0;

				let objectsCount = this.getObjectsCount(format.objects);
				let traverseObjects = (parent: BaseObject, objects: TObj[], parentObject: TObj, done: ()=>void) => {
					if (objects == null) return;
					for (let i = 0; i < objects.length; ++i) {
						let o = objects[i];
						if (o.spawn != null && o.spawn == false) {
							if (++objectsTraversed == objectsCount) done();
							continue; // Do not auto-create this object
						}

						this.createObject(o, format, parent, parentObject, (object: BaseObject) => {
							traverseObjects(object, o.children, o, done);
							if (++objectsTraversed == objectsCount) done();
						});
					}
				}

				if (format.objects == null || format.objects.length == 0) {
					done(parent);
				}
				else {
					traverseObjects(parent, format.objects, null, () => { // Scene objects
						done(parent);
					});
				}
			});
		});
	}

	getObjectsCount = (objects: TObj[], discardNoSpawn = true): i32 => {
		if (objects == null) return 0;
		let result = objects.length;
		for (let o of objects) {
			if (discardNoSpawn && o.spawn != null && o.spawn == false) continue; // Do not count children of non-spawned objects
			if (o.children != null) result += this.getObjectsCount(o.children);
		}
		return result;
	}

	spawnObject = (name: string, parent: Null<BaseObject>, done: Null<(o: BaseObject)=>void>, spawnChildren = true, srcRaw: Null<TSceneFormat> = null) => {
		if (srcRaw == null) srcRaw = this.raw;
		let objectsTraversed = 0;
		let obj = Scene.getRawObjectByName(srcRaw, name);
		let objectsCount = spawnChildren ? this.getObjectsCount([obj], false) : 1;
		let rootId = -1;
		let spawnObjectTree = (obj: TObj, parent: BaseObject, parentObject: TObj, done: (o: BaseObject)=>void) => {
			this.createObject(obj, srcRaw, parent, parentObject, (object: BaseObject) => {
				if (rootId == -1) {
					rootId = object.uid;
				}
				if (spawnChildren && obj.children != null) {
					for (let child of obj.children) spawnObjectTree(child, object, obj, done);
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

	parseObject = (sceneName: string, objectName: string, parent: BaseObject, done: (o: BaseObject)=>void) => {
		Data.getSceneRaw(sceneName, (format: TSceneFormat) => {
			let o: TObj = Scene.getRawObjectByName(format, objectName);
			if (o == null) done(null);
			this.createObject(o, format, parent, null, done);
		});
	}

	static getRawObjectByName = (format: TSceneFormat, name: string): TObj => {
		return Scene.traverseObjs(format.objects, name);
	}

	static traverseObjs = (children: TObj[], name: string): TObj => {
		for (let o of children) {
			if (o.name == name) return o;
			if (o.children != null) {
				let res = Scene.traverseObjs(o.children, name);
				if (res != null) return res;
			}
		}
		return null;
	}

	createObject = (o: TObj, format: TSceneFormat, parent: BaseObject, parentObject: TObj, done: (o: BaseObject)=>void) => {
		let sceneName = format.name;

		if (o.type == "camera_object") {
			Data.getCamera(sceneName, o.data_ref, (b: CameraData) => {
				let object = this.addCameraObject(b, parent);
				this.returnObject(object, o, done);
			});
		}
		else if (o.type == "light_object") {
			Data.getLight(sceneName, o.data_ref, (b: LightData) => {
				let object = this.addLightObject(b, parent);
				this.returnObject(object, o, done);
			});
		}
		else if (o.type == "mesh_object") {
			if (o.material_refs == null || o.material_refs.length == 0) {
				this.createMeshObject(o, format, parent, parentObject, null, done);
			}
			else {
				// Materials
				let materials: MaterialData[] = [];
				let materialsLoaded = 0;
				for (let i = 0; i < o.material_refs.length; ++i) {
					let ref = o.material_refs[i];
					Data.getMaterial(sceneName, ref, (mat: MaterialData) => {
						materials[i] = mat;
						materialsLoaded++;
						if (materialsLoaded == o.material_refs.length) {
							this.createMeshObject(o, format, parent, parentObject, materials, done);
						}
					});
				}
			}
		}
		///if arm_audio
		else if (o.type == "speaker_object") {
			let object = this.addSpeakerObject(Data.getSpeakerRawByName(format.speaker_datas, o.data_ref), parent);
			this.returnObject(object, o, done);
		}
		///end
		else if (o.type == "object") {
			let object = this.addObject(parent);
			this.returnObject(object, o, (ro: BaseObject) => {
				done(ro);
			});
		}
		else done(null);
	}

	getChildObjectsRaw = (rawObj: TObj, recursive: bool = true): TObj[] => {
		let children = rawObj.children;
		if (children == null) return [];
		children = children.slice();

		if (recursive) {
			for (let child of rawObj.children) {
				let childRefs = this.getChildObjectsRaw(child);
				children = children.concat(childRefs);
			}
		}
		return children;
	}

	createMeshObject = (o: TObj, format: TSceneFormat, parent: BaseObject, parentObject: TObj, materials: MaterialData[], done: (o: BaseObject)=>void) => {
		// Mesh reference
		let ref = o.data_ref.split("/");
		let object_file = "";
		let data_ref = "";
		let sceneName = format.name;
		if (ref.length == 2) { // File reference
			object_file = ref[0];
			data_ref = ref[1];
		}
		else { // Local mesh data
			object_file = sceneName;
			data_ref = o.data_ref;
		}

		// Bone objects are stored in armature parent
		///if arm_skin
		if (parentObject != null && parentObject.bone_actions != null) {
			let bactions: TSceneFormat[] = [];
			for (let ref of parentObject.bone_actions) {
				Data.getSceneRaw(ref, (action: TSceneFormat) => {
					bactions.push(action);
					if (bactions.length == parentObject.bone_actions.length) {
						let armature: Armature = null;
						// Check if armature exists
						for (let a of this.armatures) {
							if (a.uid == parent.uid) {
								armature = a;
								break;
							}
						}
						// Create new one
						if (armature == null) {
							// Unique name if armature was already instantiated for different object
							for (let a of this.armatures) {
								if (a.name == parent.name) {
									parent.name += "." + parent.uid;
									break;
								}
							}
							armature = new Armature(parent.uid, parent.name, bactions);
							this.armatures.push(armature);
						}
						this.returnMeshObject(
							object_file, data_ref, sceneName, armature, materials, parent, parentObject, o, done);
					}
				});
			}
			return;
		}
		///end

		this.returnMeshObject(object_file, data_ref, sceneName, null, materials, parent, parentObject, o, done);
	}

	returnMeshObject = (object_file: string, data_ref: string, sceneName: string,
		armature: any, // Armature
		materials: MaterialData[], parent: BaseObject, parentObject: TObj, o: TObj, done: (o: BaseObject)=>void) => {

			Data.getMesh(object_file, data_ref, (mesh: MeshData) => {
			///if arm_skin
			if (mesh.isSkinned) {
				let g = mesh;
				armature != null ? g.addArmature(armature) : g.addAction(mesh.format.objects, "none");
			}
			///end
			let object = this.addMeshObject(mesh, materials, parent);

			// Attach particle systems
			///if arm_particles
			if (o.particle_refs != null) {
				for (let ref of o.particle_refs) (object as MeshObject).setupParticleSystem(sceneName, ref);
			}
			///end
			this.returnObject(object, o, done);
		});
	}

	returnObject = (object: BaseObject, o: TObj, done: (o: BaseObject)=>void) => {
		// Load object actions
		if (object != null && o.object_actions != null) {
			let oactions: TSceneFormat[] = [];
			while (oactions.length < o.object_actions.length) oactions.push(null);
			let actionsLoaded = 0;
			for (let i = 0; i < o.object_actions.length; ++i) {
				let ref = o.object_actions[i];
				if (ref == "null") { // No startup action set
					actionsLoaded++;
					continue;
				}
				Data.getSceneRaw(ref, (action: TSceneFormat) => {
					oactions[i] = action;
					actionsLoaded++;
					if (actionsLoaded == o.object_actions.length) {
						this.returnObjectLoaded(object, o, oactions, done);
					}
				});
			}
		}
		else this.returnObjectLoaded(object, o, null, done);
	}

	returnObjectLoaded = (object: BaseObject, o: TObj, oactions: TSceneFormat[], done: (o: BaseObject)=>void) => {
		if (object != null) {
			object.raw = o;
			object.name = o.name;
			if (o.visible != null) object.visible = o.visible;
			if (o.visible_mesh != null) object.visibleMesh = o.visible_mesh;
			Scene.generateTransform(o, object.transform);
			object.setupAnimation(oactions);
		}
		done(object);
	}

	static generateTransform = (object: TObj, transform: Transform) => {
		transform.world = object.transform != null ? Mat4.fromFloat32Array(object.transform.values) : Mat4.identity();
		transform.world.decompose(transform.loc, transform.rot, transform.scale);
		// Whether to apply parent matrix
		if (object.local_only != null) transform.localOnly = object.local_only;
		if (transform.object.parent != null) transform.update();
	}

 	static parseArg = (str: string): any => {
		if (str == "true") return true;
		else if (str == "false") return false;
		else if (str == "null") return null;
		else if (str.charAt(0) == "'") return str.replaceAll("'", "");
		else if (str.charAt(0) == '"') return str.replaceAll('"', "");
		else if (str.charAt(0) == "[") { // Array
			// Remove [] and recursively parse into array, then append into parent
			str = str.replaceAll("[", "");
			str = str.replaceAll("]", "");
			str = str.replaceAll(" ", "");
			let ar: any = [];
			let vals = str.split(",");
			for (let v of vals) ar.push(Scene.parseArg(v));
			return ar;
		}
		else if (str.charAt(0) == '{') { // Typedef or anonymous structure
			return JSON.parse(str);
		}
		else {
			let f = parseFloat(str);
			let i = parseInt(str);
			return f == i ? i : f;
		}
	}

	static createTraitClassInstance = (traitName: string, args: any[]): any => {
		let global: any = globalThis;
		return new global[traitName](args);
	}

	loadEmbeddedData = (datas: string[], done: ()=>void) => {
		if (datas == null) {
			done();
			return;
		}
		let loaded = 0;
		for (let file of datas) {
			this.embedData(file, () => {
				loaded++;
				if (loaded == datas.length) done();
			});
		}
	}

	embedData = (file: string, done: ()=>void) => {
		if (file.endsWith(".raw")) {
			Data.getBlob(file, (b: ArrayBuffer) => {
				// Raw 3D texture bytes
				let w = Math.floor(Math.pow(b.byteLength, 1 / 3)) + 1;
				let image = Image.fromBytes3D(b, w, w, w, TextureFormat.R8);
				this.embedded.set(file, image);
				done();
			});
		}
		else {
			Data.getImage(file, (image: Image) => {
				this.embedded.set(file, image);
				done();
			});
		}
	}

	// Hooks
	notifyOnInit = (f: ()=>void) => {
		if (this.ready) f(); // Scene already running
		else this.traitInits.push(f);
	}

	removeInit = (f: ()=>void) => {
		array_remove(this.traitInits, f);
	}

	notifyOnRemove = (f: ()=>void) => {
		this.traitRemoves.push(f);
	}
}
