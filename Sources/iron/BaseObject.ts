
class TBaseObject {
	uid: i32;
	urandom: f32;
	raw: obj_t = null;
	name: string = "";
	transform: transform_t;
	traits: any[] = [];
	parent: TBaseObject = null;
	children: TBaseObject[] = [];
	animation: AnimationRaw = null;
	visible = true; // Skip render, keep updating
	culled = false; // TBaseObject was culled last frame
	isEmpty = false;
	ext: any; // TMeshObject | TCameraObject | TLightObject | TSpeakerObject
}

class BaseObject {
	static uidCounter = 0;

	static create(): TBaseObject {
		let raw = new TBaseObject();
		raw.uid = BaseObject.uidCounter++;
		raw.transform = transform_create(raw);
		raw.isEmpty = raw.constructor == TBaseObject;
		if (raw.isEmpty && _scene_ready) scene_empties.push(raw);
		return raw;
	}

	static setParent = (raw: TBaseObject, parentObject: TBaseObject, parentInverse = false, keepTransform = false) => {
		if (parentObject == raw || parentObject == raw.parent) return;

		if (raw.parent != null) {
			array_remove(raw.parent.children, raw);
			if (keepTransform) transform_apply_parent(raw.transform);
			raw.parent = null; // rebuild matrix without a parent
			transform_build_matrix(raw.transform);
		}

		if (parentObject == null) {
			parentObject = _scene_scene_parent;
		}
		raw.parent = parentObject;
		raw.parent.children.push(raw);
		if (parentInverse) transform_apply_parent_inv(raw.transform);
	}

	static removeSuper = (raw: TBaseObject) => {
		if (raw.isEmpty && _scene_ready) array_remove(scene_empties, raw);
		if (raw.animation != null) Animation.remove(raw.animation);
		while (raw.children.length > 0) BaseObject.remove(raw.children[0]);
		if (raw.parent != null) {
			array_remove(raw.parent.children, raw);
			raw.parent = null;
		}
	}

	static remove = (raw: TBaseObject) => {
		if (raw.ext != null)  {
			if (raw.ext.constructor == TMeshObject) {
				MeshObject.remove(raw.ext);
			}
			else if (raw.ext.constructor == TCameraObject) {
				CameraObject.remove(raw.ext);
			}
			else if (raw.ext.constructor == TLightObject) {
				LightObject.remove(raw.ext);
			}
			///if arm_audio
			else if (raw.ext.constructor == speaker_object_t) {
				speaker_object_remove(raw.ext);
			}
			///end
		}
		else {
			BaseObject.removeSuper(raw);
		}
	}

	static getChild = (raw: TBaseObject, name: string): TBaseObject => {
		if (raw.name == name) return raw;
		else {
			for (let c of raw.children) {
				let r = BaseObject.getChild(c, name);
				if (r != null) return r;
			}
		}
		return null;
	}

	static getChildren = (raw: TBaseObject, recursive = false): TBaseObject[] => {
		if (!recursive) return raw.children;

		let retChildren = raw.children.slice();
		for (let child of raw.children) {
			retChildren = retChildren.concat(BaseObject.getChildren(child, recursive));
		}
		return retChildren;
	}

	///if arm_skin
	static getParentArmature = (raw: TBaseObject, name: string): BoneAnimationRaw => {
		for (let a of scene_animations) if (a.armature != null && a.armature.name == name) return a.ext;
		return null;
	}
	///end

	static setupAnimationSuper = (raw: TBaseObject, oactions: scene_t[] = null) => {
		// Parented to bone
		///if arm_skin
		if (raw.raw.parent_bone != null) {
			App.notifyOnInit(() => {
				let banim = BaseObject.getParentArmature(raw, raw.parent.name);
				if (banim != null) BoneAnimation.addBoneChild(banim, raw.raw.parent_bone, raw);
			});
		}
		///end
		// TBaseObject actions
		if (oactions == null) return;
		raw.animation = ObjectAnimation.create(raw, oactions).base;
	}

	static setupAnimation = (raw: TBaseObject, oactions: scene_t[] = null) => {
		if (raw.ext != null)  {
			if (raw.ext.constructor == TMeshObject) {
				MeshObject.setupAnimation(raw.ext, oactions);
			}
		}
		else {
			BaseObject.setupAnimationSuper(raw, oactions);
		}
	}
}
