package iron.object;

import iron.data.SceneFormat;

class Object {
	static var uidCounter = 0;
	public var uid: Int;
	public var urandom: Float;
	public var raw: TObj = null;

	public var name: String = "";
	public var transform: Transform;
	public var traits: Array<Dynamic> = [];

	public var parent: Object = null;
	public var children: Array<Object> = [];

	public var animation: Animation = null;
	public var visible = true; // Skip render, keep updating
	public var visibleMesh = true;
	public var culled = false; // Object was culled last frame
	public var culledMesh = false;
	var isEmpty = false;

	public function new() {
		uid = uidCounter++;
		transform = new Transform(this);
		isEmpty = Type.getClass(this) == Object;
		if (isEmpty && Scene.active != null) Scene.active.empties.push(this);
	}

	public function setParent(parentObject: Object, parentInverse = false, keepTransform = false) {
		if (parentObject == this || parentObject == parent) return;

		if (parent != null) {
			parent.children.remove(this);
			if (keepTransform) this.transform.applyParent();
			this.parent = null; // rebuild matrix without a parent
			this.transform.buildMatrix();
		}

		if (parentObject == null) {
			parentObject = Scene.active.sceneParent;
		}
		parent = parentObject;
		parent.children.push(this);
		if (parentInverse) this.transform.applyParentInverse();
	}

	public function remove() {
		if (isEmpty && Scene.active != null) Scene.active.empties.remove(this);
		if (animation != null) animation.remove();
		while (children.length > 0) children[0].remove();
		while (traits.length > 0) traits[0].remove();
		if (parent != null) {
			parent.children.remove(this);
			parent = null;
		}
	}

	public function getChild(name: String): Object {
		if (this.name == name) return this;
		else {
			for (c in children) {
				var r = c.getChild(name);
				if (r != null) return r;
			}
		}
		return null;
	}

	public function getChildren(?recursive = false): Array<Object> {
		if (!recursive) return children;

		var retChildren = children.copy();
		for (child in children) {
			retChildren = retChildren.concat(child.getChildren(recursive));
		}
		return retChildren;
	}

	public function getChildOfType<T: Object>(type: Class<T>): T {
		if (Std.isOfType(this, type)) return cast this;
		else {
			for (c in children) {
				var r = c.getChildOfType(type);
				if (r != null) return r;
			}
		}
		return null;
	}

	public function addTrait(t: Dynamic) {
		traits.push(t);
		t.object = this;

		// if (t._add != null) {
		// 	for (f in t._add) f();
		// 	t._add = null;
		// }
	}

	public function removeTrait(t: Dynamic) {
		t.remove();
		traits.remove(t);
	}

	public function getTrait(c: Class<Dynamic>): Dynamic {
		for (t in traits) if (Type.getClass(t) == cast c) return cast t;
		return null;
	}

	#if arm_skin
	public function getParentArmature(name: String): BoneAnimation {
		for (a in Scene.active.animations) if (a.armature != null && a.armature.name == name) return cast a;
		return null;
	}
	#end

	public function setupAnimation(oactions: Array<TSceneFormat> = null) {
		// Parented to bone
		#if arm_skin
		if (raw.parent_bone != null) {
			Scene.active.notifyOnInit(function() {
				var banim = getParentArmature(parent.name);
				if (banim != null) banim.addBoneChild(raw.parent_bone, this);
			});
		}
		#end
		// Object actions
		if (oactions == null) return;
		animation = new ObjectAnimation(this, oactions);
	}
}
