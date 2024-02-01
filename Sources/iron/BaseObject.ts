
class TBaseObject {
	uid: i32;
	urandom: f32;
	raw: TObj = null;
	name: string = "";
	transform: TransformRaw;
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
		raw.transform = Transform.create(raw);
		raw.isEmpty = raw.constructor == TBaseObject;
		if (raw.isEmpty && Scene.ready) Scene.empties.push(raw);
		return raw;
	}

	static setParent = (raw: TBaseObject, parentObject: TBaseObject, parentInverse = false, keepTransform = false) => {
		if (parentObject == raw || parentObject == raw.parent) return;

		if (raw.parent != null) {
			array_remove(raw.parent.children, raw);
			if (keepTransform) Transform.applyParent(raw.transform);
			raw.parent = null; // rebuild matrix without a parent
			Transform.buildMatrix(raw.transform);
		}

		if (parentObject == null) {
			parentObject = Scene.sceneParent;
		}
		raw.parent = parentObject;
		raw.parent.children.push(raw);
		if (parentInverse) Transform.applyParentInverse(raw.transform);
	}

	static removeSuper = (raw: TBaseObject) => {
		if (raw.isEmpty && Scene.ready) array_remove(Scene.empties, raw);
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
			else if (raw.ext.constructor == TSpeakerObject) {
				SpeakerObject.remove(raw.ext);
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
		for (let a of Scene.animations) if (a.armature != null && a.armature.name == name) return a.ext;
		return null;
	}
	///end

	static setupAnimationSuper = (raw: TBaseObject, oactions: TSceneFormat[] = null) => {
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

	static setupAnimation = (raw: TBaseObject, oactions: TSceneFormat[] = null) => {
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
