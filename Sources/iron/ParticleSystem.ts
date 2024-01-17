
///if arm_particles

class ParticleSystem {
	data: ParticleData;
	speed = 1.0;
	particles: Particle[];
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

	constructor(sceneName: string, pref: TParticleReference) {
		this.seed = pref.seed;
		this.particles = [];
		this.ready = false;
		Data.getParticle(sceneName, pref.particle, (b: ParticleData) => {
			this.data = b;
			this.r = this.data.raw;
			this.gx = 0;
			this.gy = 0;
			this.gz = -9.81 * this.r.weight_gravity;
			this.alignx = this.r.object_align_factor[0] / 2;
			this.aligny = this.r.object_align_factor[1] / 2;
			this.alignz = this.r.object_align_factor[2] / 2;
			this.lifetime = this.r.lifetime / this.frameRate;
			this.animtime = (this.r.frame_end - this.r.frame_start) / this.frameRate;
			this.spawnRate = ((this.r.frame_end - this.r.frame_start) / this.r.count) / this.frameRate;
			for (let i = 0; i < this.r.count; ++i) this.particles.push(new Particle(i));
			this.ready = true;
		});
	}

	pause = () => {
		this.lifetime = 0;
	}

	resume = () => {
		this.lifetime = this.r.lifetime / this.frameRate;
	}

	update = (object: MeshObject, owner: MeshObject) => {
		if (!this.ready || object == null || this.speed == 0.0) return;

		// Copy owner world transform but discard scale
		owner.transform.world.decompose(this.ownerLoc, this.ownerRot, this.ownerScl);
		object.transform.loc = this.ownerLoc;
		object.transform.rot = this.ownerRot;

		// Set particle size per particle system
		object.transform.scale = new Vec4(this.r.particle_size, this.r.particle_size, this.r.particle_size, 1);

		object.transform.buildMatrix();
		owner.transform.buildMatrix();
		object.transform.dim.setFrom(owner.transform.dim);

		this.dimx = object.transform.dim.x;
		this.dimy = object.transform.dim.y;

		// Animate
		this.time += Time.realDelta * this.speed;
		this.lap = Math.floor(this.time / this.animtime);
		this.lapTime = this.time - this.lap * this.animtime;
		this.count = Math.floor(this.lapTime / this.spawnRate);

		this.updateGpu(object, owner);
	}

	getData = (): Mat4 => {
		let hair = this.r.type == 1;
		this.m._00 = this.r.loop ? this.animtime : -this.animtime;
		this.m._01 = hair ? 1 / this.particles.length : this.spawnRate;
		this.m._02 = hair ? 1 : this.lifetime;
		this.m._03 = this.particles.length;
		this.m._10 = hair ? 0 : this.alignx;
		this.m._11 = hair ? 0 : this.aligny;
		this.m._12 = hair ? 0 : this.alignz;
		this.m._13 = hair ? 0 : this.r.factor_random;
		this.m._20 = hair ? 0 : this.gx * this.r.mass;
		this.m._21 = hair ? 0 : this.gy * this.r.mass;
		this.m._22 = hair ? 0 : this.gz * this.r.mass;
		this.m._23 = hair ? 0 : this.r.lifetime_random;
		this.m._30 = 1; // tilesx
		this.m._31 = 1; // tilesy
		this.m._32 = 1; // tilesframerate
		this.m._33 = hair ? 1 : this.lapTime;
		return this.m;
	}

	updateGpu = (object: MeshObject, owner: MeshObject) => {
		if (!object.data.instanced) this.setupGeomGpu(object, owner);
		// GPU particles transform is attached to owner object
	}

	rand = (max: i32): i32 => {
		return Math.floor(Math.random() * max);
	}

	setupGeomGpu = (object: MeshObject, owner: MeshObject) => {
		let instancedData = new Float32Array(this.particles.length * 3);
		let i = 0;

		let normFactor = 1 / 32767; // pa.values are not normalized
		let scalePosOwner = owner.data.scalePos;
		let scalePosParticle = object.data.scalePos;
		let particleSize = this.r.particle_size;
		let scaleFactor = new Vec4().setFrom(owner.transform.scale);
		scaleFactor.mult(scalePosOwner / (particleSize * scalePosParticle));

		switch (this.r.emit_from) {
			case 0: // Vert
				let pa = owner.data.positions;

				for (let p of this.particles) {
					let j = Math.floor(this.fhash(i) * (pa.values.length / pa.size));
					instancedData[i] = pa.values[j * pa.size    ] * normFactor * scaleFactor.x; i++;
					instancedData[i] = pa.values[j * pa.size + 1] * normFactor * scaleFactor.y; i++;
					instancedData[i] = pa.values[j * pa.size + 2] * normFactor * scaleFactor.z; i++;
				}

			case 1: // Face
				let positions = owner.data.positions.values;

				for (let p of this.particles) {
					// Choose random index array (there is one per material) and random face
					let ia = owner.data.indices[this.rand(owner.data.indices.length)];
					let faceIndex = this.rand(Math.floor(ia.length / 3));

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
				let scaleFactorVolume = new Vec4().setFrom(object.transform.dim);
				scaleFactorVolume.mult(0.5 / (particleSize * scalePosParticle));

				for (let p in this.particles) {
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.x; i++;
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.y; i++;
					instancedData[i] = (Math.random() * 2.0 - 1.0) * scaleFactorVolume.z; i++;
				}
		}
		object.data.setupInstanced(instancedData, 1, Usage.StaticUsage);
	}

	fhash = (n: i32): f32 => {
		let s = n + 1.0;
		s *= 9301.0 % s;
		s = (s * 9301.0 + 49297.0) % 233280.0;
		return s / 233280.0;
	}

	remove = () => {}

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

class Particle {
	i: i32;
	x = 0.0;
	y = 0.0;
	z = 0.0;
	cameraDistance: f32;

	constructor(i: i32) {
		this.i = i;
	}
}

///end
