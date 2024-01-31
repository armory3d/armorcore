
///if arm_particles

class TParticleSystem {
	data: TParticleData;
	speed = 1.0;
	particles: TParticle[];
	ready: bool;
	frameRate = 24;
	lifetime = 0.0;
	animtime = 0.0;
	time = 0.0;
	spawnRate = 0.0;
	seed = 0;

	r: TParticleData;
	gx: f32;
	gy: f32;
	gz: f32;
	alignx: f32;
	aligny: f32;
	alignz: f32;
	dimx: f32;
	dimy: f32;

	count = 0;
	lap = 0;
	lapTime = 0.0;
	m = Mat4.identity();

	ownerLoc = new Vec4();
	ownerRot = new Quat();
	ownerScl = new Vec4();
}

class ParticleSystem {

	static create(sceneName: string, pref: TParticleReference): TParticleSystem {
		let raw = new TParticleSystem();
		raw.seed = pref.seed;
		raw.particles = [];
		raw.ready = false;
		Data.getParticle(sceneName, pref.particle, (b: TParticleData) => {
			raw.data = b;
			raw.r = raw.data;
			raw.gx = 0;
			raw.gy = 0;
			raw.gz = -9.81 * raw.r.weight_gravity;
			raw.alignx = raw.r.object_align_factor[0] / 2;
			raw.aligny = raw.r.object_align_factor[1] / 2;
			raw.alignz = raw.r.object_align_factor[2] / 2;
			raw.lifetime = raw.r.lifetime / raw.frameRate;
			raw.animtime = (raw.r.frame_end - raw.r.frame_start) / raw.frameRate;
			raw.spawnRate = ((raw.r.frame_end - raw.r.frame_start) / raw.r.count) / raw.frameRate;
			for (let i = 0; i < raw.r.count; ++i) {
				let p = new TParticle();
				p.i = i;
				raw.particles.push(p);
			}
			raw.ready = true;
		});
		return raw;
	}

	static pause = (raw: TParticleSystem) => {
		raw.lifetime = 0;
	}

	static resume = (raw: TParticleSystem) => {
		raw.lifetime = raw.r.lifetime / raw.frameRate;
	}

	static update = (raw: TParticleSystem, object: MeshObject, owner: MeshObject) => {
		if (!raw.ready || object == null || raw.speed == 0.0) return;

		// Copy owner world transform but discard scale
		owner.base.transform.world.decompose(raw.ownerLoc, raw.ownerRot, raw.ownerScl);
		object.base.transform.loc = raw.ownerLoc;
		object.base.transform.rot = raw.ownerRot;

		// Set particle size per particle system
		object.base.transform.scale = new Vec4(raw.r.particle_size, raw.r.particle_size, raw.r.particle_size, 1);

		object.base.transform.buildMatrix();
		owner.base.transform.buildMatrix();
		object.base.transform.dim.setFrom(owner.base.transform.dim);

		raw.dimx = object.base.transform.dim.x;
		raw.dimy = object.base.transform.dim.y;

		// Animate
		raw.time += Time.realDelta * raw.speed;
		raw.lap = Math.floor(raw.time / raw.animtime);
		raw.lapTime = raw.time - raw.lap * raw.animtime;
		raw.count = Math.floor(raw.lapTime / raw.spawnRate);

		ParticleSystem.updateGpu(raw, object, owner);
	}

	static getData = (raw: TParticleSystem): Mat4 => {
		let hair = raw.r.type == 1;
		raw.m._00 = raw.r.loop ? raw.animtime : -raw.animtime;
		raw.m._01 = hair ? 1 / raw.particles.length : raw.spawnRate;
		raw.m._02 = hair ? 1 : raw.lifetime;
		raw.m._03 = raw.particles.length;
		raw.m._10 = hair ? 0 : raw.alignx;
		raw.m._11 = hair ? 0 : raw.aligny;
		raw.m._12 = hair ? 0 : raw.alignz;
		raw.m._13 = hair ? 0 : raw.r.factor_random;
		raw.m._20 = hair ? 0 : raw.gx * raw.r.mass;
		raw.m._21 = hair ? 0 : raw.gy * raw.r.mass;
		raw.m._22 = hair ? 0 : raw.gz * raw.r.mass;
		raw.m._23 = hair ? 0 : raw.r.lifetime_random;
		raw.m._30 = 1; // tilesx
		raw.m._31 = 1; // tilesy
		raw.m._32 = 1; // tilesframerate
		raw.m._33 = hair ? 1 : raw.lapTime;
		return raw.m;
	}

	static updateGpu = (raw: TParticleSystem, object: MeshObject, owner: MeshObject) => {
		if (!object.data._instanced) ParticleSystem.setupGeomGpu(raw, object, owner);
		// GPU particles transform is attached to owner object
	}

	static rand = (max: i32): i32 => {
		return Math.floor(Math.random() * max);
	}

	static setupGeomGpu = (raw: TParticleSystem, object: MeshObject, owner: MeshObject) => {
		let instancedData = new Float32Array(raw.particles.length * 3);
		let i = 0;

		let normFactor = 1 / 32767; // pa.values are not normalized
		let scalePosOwner = owner.data.scale_pos;
		let scalePosParticle = object.data.scale_pos;
		let particleSize = raw.r.particle_size;
		let scaleFactor = new Vec4().setFrom(owner.base.transform.scale);
		scaleFactor.mult(scalePosOwner / (particleSize * scalePosParticle));

		switch (raw.r.emit_from) {
			case 0: // Vert
				let pa = MeshData.getVArray(owner.data, 'pos');

				for (let p of raw.particles) {
					let j = Math.floor(ParticleSystem.fhash(i) * (pa.values.length / pa._size));
					instancedData[i] = pa.values[j * pa._size    ] * normFactor * scaleFactor.x; i++;
					instancedData[i] = pa.values[j * pa._size + 1] * normFactor * scaleFactor.y; i++;
					instancedData[i] = pa.values[j * pa._size + 2] * normFactor * scaleFactor.z; i++;
				}

			case 1: // Face
				let positions = MeshData.getVArray(owner.data, 'pos').values;

				for (let p of raw.particles) {
					// Choose random index array (there is one per material) and random face
					let ia = owner.data._indices[ParticleSystem.rand(owner.data._indices.length)];
					let faceIndex = ParticleSystem.rand(Math.floor(ia.length / 3));

					let i0 = ia[faceIndex * 3 + 0];
					let i1 = ia[faceIndex * 3 + 1];
					let i2 = ia[faceIndex * 3 + 2];

					let v0 = new Vec3(positions[i0 * 4], positions[i0 * 4 + 1], positions[i0 * 4 + 2]);
					let v1 = new Vec3(positions[i1 * 4], positions[i1 * 4 + 1], positions[i1 * 4 + 2]);
					let v2 = new Vec3(positions[i2 * 4], positions[i2 * 4 + 1], positions[i2 * 4 + 2]);

					let pos = ParticleSystem.randomPointInTriangle(v0, v1, v2);

					instancedData[i] = pos.x * normFactor * scaleFactor.x; i++;
					instancedData[i] = pos.y * normFactor * scaleFactor.y; i++;
					instancedData[i] = pos.z * normFactor * scaleFactor.z; i++;
				}

			case 2: // Volume
				let scaleFactorVolume = new Vec4().setFrom(object.base.transform.dim);
				scaleFactorVolume.mult(0.5 / (particleSize * scalePosParticle));

				for (let p in raw.particles) {
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.x; i++;
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.y; i++;
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.z; i++;
				}
		}
		MeshData.setupInstanced(object.data, instancedData, 1);
	}

	static fhash = (n: i32): f32 => {
		let s = n + 1.0;
		s *= 9301.0 % s;
		s = (s * 9301.0 + 49297.0) % 233280.0;
		return s / 233280.0;
	}

	static remove = (raw: TParticleSystem) => {}

	static randomPointInTriangle = (a: Vec3, b: Vec3, c: Vec3): Vec3 => {
		// Generate a random point in a square where (0, 0) <= (x, y) < (1, 1)
		let x = Math.random();
		let y = Math.random();

		if (x + y > 1) {
			// We're in the upper right triangle in the square, mirror to lower left
			x = 1 - x;
			y = 1 - y;
		}

		// Transform the point to the triangle abc
		let u = b.sub(a);
		let v = c.sub(a);
		return a.add(u.mult(x).add(v.mult(y)));
	}
}

class TParticle {
	i: i32;
	x = 0.0;
	y = 0.0;
	z = 0.0;
	cameraDistance: f32;
}

///end
