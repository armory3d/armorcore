
class BaseObject {
	static uidCounter = 0;
	uid: i32;
	urandom: f32;
	raw: TObj = null;

	name: string = "";
	transform: Transform;
	traits: any[] = [];

	parent: BaseObject = null;
	children: BaseObject[] = [];

	animation: Animation = null;
	visible = true; // Skip render, keep updating
	culled = false; // BaseObject was culled last frame
	isEmpty = false;

	constructor() {
		this.uid = BaseObject.uidCounter++;
		this.transform = new Transform(this);
		this.isEmpty = this.constructor == BaseObject;
		if (this.isEmpty && Scene.active != null) Scene.active.empties.push(this);
	}

	setParent = (parentObject: BaseObject, parentInverse = false, keepTransform = false) => {
		if (parentObject == this || parentObject == this.parent) return;

		if (this.parent != null) {
			array_remove(this.parent.children, this);
			if (keepTransform) this.transform.applyParent();
			this.parent = null; // rebuild matrix without a parent
			this.transform.buildMatrix();
		}

		if (parentObject == null) {
			parentObject = Scene.active.sceneParent;
		}
		this.parent = parentObject;
		this.parent.children.push(this);
		if (parentInverse) this.transform.applyParentInverse();
	}

	removeSuper = () => {
		if (this.isEmpty && Scene.active != null) array_remove(Scene.active.empties, this);
		if (this.animation != null) this.animation.remove();
		while (this.children.length > 0) this.children[0].remove();
		if (this.parent != null) {
			array_remove(this.parent.children, this);
			this.parent = null;
		}
	}

	remove = this.removeSuper;

	getChild = (name: string): BaseObject => {
		if (this.name == name) return this;
		else {
			for (let c of this.children) {
				let r = c.getChild(name);
				if (r != null) return r;
			}
		}
		return null;
	}

	getChildren = (recursive = false): BaseObject[] => {
		if (!recursive) return this.children;

		let retChildren = this.children.slice();
		for (let child of this.children) {
			retChildren = retChildren.concat(child.getChildren(recursive));
		}
		return retChildren;
	}

	///if arm_skin
	getParentArmature = (name: string): BoneAnimation => {
		for (let a of Scene.active.animations) if (a.armature != null && a.armature.name == name) return a as BoneAnimation;
		return null;
	}
	///end

	setupAnimationSuper = (oactions: TSceneFormat[] = null) => {
		// Parented to bone
		///if arm_skin
		if (this.raw.parent_bone != null) {
			Scene.active.notifyOnInit(() => {
				let banim = this.getParentArmature(this.parent.name);
				if (banim != null) banim.addBoneChild(this.raw.parent_bone, this);
			});
		}
		///end
		// BaseObject actions
		if (oactions == null) return;
		this.animation = new ObjectAnimation(this, oactions);
	}

	setupAnimation = this.setupAnimationSuper;
}
