
class Scene {

	static ready: bool;
	static uidCounter = 0;
	static uid: i32;
	static raw: TSceneFormat;
	static root: TBaseObject;
	static sceneParent: TBaseObject;
	static camera: TCameraObject;
	static world: TWorldData;

	static meshes: TMeshObject[];
	static lights: TLightObject[];
	static cameras: TCameraObject[];
	///if arm_audio
	static speakers: TSpeakerObject[];
	///end
	static empties: TBaseObject[];
	static animations: AnimationRaw[];
	///if arm_skin
	static armatures: TArmature[];
	///end
	static embedded: Map<string, ImageRaw>;

	static create = (format: TSceneFormat, done: (o: TBaseObject)=>void) => {
		Scene.uid = Scene.uidCounter++;
		Scene.meshes = [];
		Scene.lights = [];
		Scene.cameras = [];
		///if arm_audio
		Scene.speakers = [];
		///end
		Scene.empties = [];
		Scene.animations = [];
		///if arm_skin
		Scene.armatures = [];
		///end
		Scene.embedded = new Map();
		Scene.root = BaseObject.create();
		Scene.root.name = "Root";

		Scene.ready = false;
		Scene.raw = format;

		Data.getWorld(format.name, format.world_ref, (world: TWorldData) => {
			Scene.world = world;

			// Startup scene
			Scene.addScene(format.name, null, (sceneObject: TBaseObject) => {
				if (Scene.cameras.length == 0) {
					Krom.log('No camera found for scene "' + format.name + '"');
				}

				Scene.camera = Scene.getCamera(format.camera_ref);
				Scene.sceneParent = sceneObject;
				Scene.ready = true;
				done(sceneObject);
			});
		});
	}

	static remove = () => {
		for (let o of Scene.meshes) MeshObject.remove(o);
		for (let o of Scene.lights) LightObject.remove(o);
		for (let o of Scene.cameras) CameraObject.remove(o);
		///if arm_audio
		for (let o of Scene.speakers) SpeakerObject.remove(o);
		///end
		for (let o of Scene.empties) BaseObject.remove(o);
		BaseObject.remove(Scene.root);
	}

	static setActive = (sceneName: string, done: (o: TBaseObject)=>void = null) => {
		if (Scene.root != null) {
			Scene.remove();
		}

		Data.getSceneRaw(sceneName, (format: TSceneFormat) => {
			Scene.create(format, (o: TBaseObject) => {
				if (done != null) done(o);
				///if arm_voxels // Revoxelize
				RenderPath.voxelized = 0;
				///end
			});
		});
	}

	static updateFrame = () => {
		if (!Scene.ready) return;
		for (let anim of Scene.animations) Animation.update(anim, Time.delta);
		for (let e of Scene.empties) if (e != null && e.parent != null) Transform.update(e.transform);
	}

	static renderFrame = (g: Graphics4Raw) => {
		if (!Scene.ready || RenderPath.commands == null) return;

		// Render active camera
		Scene.camera != null ? CameraObject.renderFrame(Scene.camera, g) : RenderPath.renderFrame(g);
	}

	// Objects
	static addObject = (parent: TBaseObject = null): TBaseObject => {
		let object = BaseObject.create();
		parent != null ? BaseObject.setParent(object, parent) : BaseObject.setParent(object, Scene.root);
		return object;
	}

	static getChild = (name: string): TBaseObject => {
		return BaseObject.getChild(Scene.root, name);
	}

	static getMesh = (name: string): TMeshObject => {
		for (let m of Scene.meshes) if (m.base.name == name) return m;
		return null;
	}

	static getLight = (name: string): TLightObject => {
		for (let l of Scene.lights) if (l.base.name == name) return l;
		return null;
	}

	static getCamera = (name: string): TCameraObject => {
		for (let c of Scene.cameras) if (c.base.name == name) return c;
		return null;
	}

	///if arm_audio
	static getSpeaker = (name: string): TSpeakerObject => {
		for (let s of Scene.speakers) if (s.base.name == name) return s;
		return null;
	}
	///end

	static getEmpty = (name: string): TBaseObject => {
		for (let e of Scene.empties) if (e.name == name) return e;
		return null;
	}

	static addMeshObject = (data: TMeshData, materials: TMaterialData[], parent: TBaseObject = null): TMeshObject => {
		let object = MeshObject.create(data, materials);
		parent != null ? BaseObject.setParent(object.base, parent) : BaseObject.setParent(object.base, Scene.root);
		return object;
	}

	static addLightObject = (data: TLightData, parent: TBaseObject = null): TLightObject => {
		let object = LightObject.create(data);
		parent != null ? BaseObject.setParent(object.base, parent) : BaseObject.setParent(object.base, Scene.root);
		return object;
	}

	static addCameraObject = (data: TCameraData, parent: TBaseObject = null): TCameraObject => {
		let object = CameraObject.create(data);
		parent != null ? BaseObject.setParent(object.base, parent) : BaseObject.setParent(object.base, Scene.root);
		return object;
	}

	///if arm_audio
	static addSpeakerObject = (data: TSpeakerData, parent: TBaseObject = null): TSpeakerObject => {
		let object = SpeakerObject.create(data);
		parent != null ? BaseObject.setParent(object.base, parent) : BaseObject.setParent(object.base, Scene.root);
		return object;
	}
	///end

	static addScene = (sceneName: string, parent: TBaseObject, done: (o: TBaseObject)=>void) => {
		if (parent == null) {
			parent = Scene.addObject();
			parent.name = sceneName;
		}
		Data.getSceneRaw(sceneName, (format: TSceneFormat) => {
			Scene.loadEmbeddedData(format.embedded_datas, () => { // Additional scene assets
				let objectsTraversed = 0;

				let objectsCount = Scene.getObjectsCount(format.objects);
				let traverseObjects = (parent: TBaseObject, objects: TObj[], parentObject: TObj, done: ()=>void) => {
					if (objects == null) return;
					for (let i = 0; i < objects.length; ++i) {
						let o = objects[i];
						if (o.spawn != null && o.spawn == false) {
							if (++objectsTraversed == objectsCount) done();
							continue; // Do not auto-create Scene object
						}

						Scene.createObject(o, format, parent, parentObject, (object: TBaseObject) => {
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

	static getObjectsCount = (objects: TObj[], discardNoSpawn = true): i32 => {
		if (objects == null) return 0;
		let result = objects.length;
		for (let o of objects) {
			if (discardNoSpawn && o.spawn != null && o.spawn == false) continue; // Do not count children of non-spawned objects
			if (o.children != null) result += Scene.getObjectsCount(o.children);
		}
		return result;
	}

	static spawnObject = (name: string, parent: Null<TBaseObject>, done: Null<(o: TBaseObject)=>void>, spawnChildren = true, srcRaw: Null<TSceneFormat> = null) => {
		if (srcRaw == null) srcRaw = Scene.raw;
		let objectsTraversed = 0;
		let obj = Scene.getRawObjectByName(srcRaw, name);
		let objectsCount = spawnChildren ? Scene.getObjectsCount([obj], false) : 1;
		let rootId = -1;
		let spawnObjectTree = (obj: TObj, parent: TBaseObject, parentObject: TObj, done: (o: TBaseObject)=>void) => {
			Scene.createObject(obj, srcRaw, parent, parentObject, (object: TBaseObject) => {
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

	static createObject = (o: TObj, format: TSceneFormat, parent: TBaseObject, parentObject: TObj, done: (o: TBaseObject)=>void) => {
		let sceneName = format.name;

		if (o.type == "camera_object") {
			Data.getCamera(sceneName, o.data_ref, (b: TCameraData) => {
				let object = Scene.addCameraObject(b, parent);
				Scene.returnObject(object.base, o, done);
			});
		}
		else if (o.type == "light_object") {
			Data.getLight(sceneName, o.data_ref, (b: TLightData) => {
				let object = Scene.addLightObject(b, parent);
				Scene.returnObject(object.base, o, done);
			});
		}
		else if (o.type == "mesh_object") {
			if (o.material_refs == null || o.material_refs.length == 0) {
				Scene.createMeshObject(o, format, parent, parentObject, null, done);
			}
			else {
				// Materials
				let materials: TMaterialData[] = [];
				let materialsLoaded = 0;
				for (let i = 0; i < o.material_refs.length; ++i) {
					let ref = o.material_refs[i];
					Data.getMaterial(sceneName, ref, (mat: TMaterialData) => {
						materials[i] = mat;
						materialsLoaded++;
						if (materialsLoaded == o.material_refs.length) {
							Scene.createMeshObject(o, format, parent, parentObject, materials, done);
						}
					});
				}
			}
		}
		///if arm_audio
		else if (o.type == "speaker_object") {
			let object = Scene.addSpeakerObject(Data.getSpeakerRawByName(format.speaker_datas, o.data_ref), parent);
			Scene.returnObject(object.base, o, done);
		}
		///end
		else if (o.type == "object") {
			let object = Scene.addObject(parent);
			Scene.returnObject(object, o, (ro: TBaseObject) => {
				done(ro);
			});
		}
		else done(null);
	}

	static createMeshObject = (o: TObj, format: TSceneFormat, parent: TBaseObject, parentObject: TObj, materials: TMaterialData[], done: (o: TBaseObject)=>void) => {
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
						let armature: TArmature = null;
						// Check if armature exists
						for (let a of Scene.armatures) {
							if (a.uid == parent.uid) {
								armature = a;
								break;
							}
						}
						// Create new one
						if (armature == null) {
							// Unique name if armature was already instantiated for different object
							for (let a of Scene.armatures) {
								if (a.name == parent.name) {
									parent.name += "." + parent.uid;
									break;
								}
							}
							armature = Armature.create(parent.uid, parent.name, bactions);
							Scene.armatures.push(armature);
						}
						Scene.returnMeshObject(
							object_file, data_ref, sceneName, armature, materials, parent, parentObject, o, done);
					}
				});
			}
			return;
		}
		///end

		Scene.returnMeshObject(object_file, data_ref, sceneName, null, materials, parent, parentObject, o, done);
	}

	static returnMeshObject = (object_file: string, data_ref: string, sceneName: string, armature: any, // TArmature
		materials: TMaterialData[], parent: TBaseObject, parentObject: TObj, o: TObj, done: (o: TBaseObject)=>void) => {

			Data.getMesh(object_file, data_ref, (mesh: TMeshData) => {
			///if arm_skin
			if (mesh.skin != null) {
				armature != null ? MeshData.addArmature(mesh, armature) : MeshData.addAction(mesh, Scene.raw.objects, "none");
			}
			///end
			let object = Scene.addMeshObject(mesh, materials, parent);

			// Attach particle systems
			///if arm_particles
			if (o.particle_refs != null) {
				for (let ref of o.particle_refs) {
					MeshObject.setupParticleSystem(object, sceneName, ref);
				}
			}
			///end
			Scene.returnObject(object.base, o, done);
		});
	}

	static returnObject = (object: TBaseObject, o: TObj, done: (o: TBaseObject)=>void) => {
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
						Scene.returnObjectLoaded(object, o, oactions, done);
					}
				});
			}
		}
		else Scene.returnObjectLoaded(object, o, null, done);
	}

	static returnObjectLoaded = (object: TBaseObject, o: TObj, oactions: TSceneFormat[], done: (o: TBaseObject)=>void) => {
		if (object != null) {
			object.raw = o;
			object.name = o.name;
			if (o.visible != null) object.visible = o.visible;
			Scene.generateTransform(o, object.transform);
			BaseObject.setupAnimation(object, oactions);
		}
		done(object);
	}

	static generateTransform = (object: TObj, transform: TransformRaw) => {
		transform.world = object.transform != null ? Mat4.fromFloat32Array(object.transform.values) : Mat4.identity();
		Mat4.decompose(transform.world, transform.loc, transform.rot, transform.scale);
		// Whether to apply parent matrix
		if (object.local_only != null) transform.localOnly = object.local_only;
		if (transform.object.parent != null) Transform.update(transform);
	}

	static loadEmbeddedData = (datas: string[], done: ()=>void) => {
		if (datas == null) {
			done();
			return;
		}
		let loaded = 0;
		for (let file of datas) {
			Scene.embedData(file, () => {
				loaded++;
				if (loaded == datas.length) done();
			});
		}
	}

	static embedData = (file: string, done: ()=>void) => {
		if (file.endsWith(".raw")) {
			Data.getBlob(file, (b: ArrayBuffer) => {
				// Raw 3D texture bytes
				let w = Math.floor(Math.pow(b.byteLength, 1 / 3)) + 1;
				let image = Image.fromBytes3D(b, w, w, w, TextureFormat.R8);
				Scene.embedded.set(file, image);
				done();
			});
		}
		else {
			Data.getImage(file, (image: ImageRaw) => {
				Scene.embedded.set(file, image);
				done();
			});
		}
	}
}
