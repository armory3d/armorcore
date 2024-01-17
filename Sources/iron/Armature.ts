
///if arm_skin

class Armature {
	uid: i32;
	name: string;
	actions: TAction[] = [];
	matsReady = false;

	constructor(uid: i32, name: string, actions: TSceneFormat[]) {
		this.uid = uid;
		this.name = name;

		for (let a of actions) {
			for (let o of a.objects) Armature.setParents(o);
			let bones: TObj[] = [];
			Armature.traverseBones(a.objects, (object: TObj) => { bones.push(object); });
			this.actions.push({ name: a.name, bones: bones, mats: null });
		}
	}

	initMats = () => {
		if (this.matsReady) return;
		this.matsReady = true;

		for (let a of this.actions) {
			if (a.mats != null) continue;
			a.mats = [];
			for (let b of a.bones) {
				a.mats.push(Mat4.fromFloat32Array(b.transform.values));
			}
		}
	}

	getAction = (name: string): TAction => {
		for (let a of this.actions) if (a.name == name) return a;
		return null;
	}

	static setParents = (object: TObj) => {
		if (object.children == null) return;
		for (let o of object.children) {
			o.parent = object;
			Armature.setParents(o);
		}
	}

	static traverseBones = (objects: TObj[], callback: (o: TObj)=>void) => {
		for (let i = 0; i < objects.length; ++i) {
			Armature.traverseBonesStep(objects[i], callback);
		}
	}

	static traverseBonesStep = (object: TObj, callback: (o: TObj)=>void) => {
		if (object.type == "bone_object") callback(object);
		if (object.children == null) return;
		for (let i = 0; i < object.children.length; ++i) {
			Armature.traverseBonesStep(object.children[i], callback);
		}
	}
}

type TAction = {
	name: string;
	bones: TObj[];
	mats: Mat4[];
}

///end
