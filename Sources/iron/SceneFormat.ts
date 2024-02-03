
type TSceneFormat = {
	name?: string;
	mesh_datas?: TMeshData[];
	light_datas?: TLightData[];
	camera_datas?: TCameraData[];
	camera_ref?: string; // Active camera
	material_datas?: TMaterialData[];
	particle_datas?: TParticleData[];
	shader_datas?: TShaderData[];
	speaker_datas?: TSpeakerData[];
	world_datas?: TWorldData[];
	world_ref?: string;
	objects?: TObj[];
	embedded_datas?: string[]; // Preload for this scene, images only for now
}

type TMeshData = {
	name: string;
	vertex_arrays: TVertexArray[];
	index_arrays: TIndexArray[];
	skin?: TSkin;
	instanced_data?: Float32Array;
	instanced_type?: Null<i32>; // off, loc, loc+rot, loc+scale, loc+rot+scale
	scale_pos?: Null<f32>; // Unpack pos from (-1,1) coords
	scale_tex?: Null<f32>; // Unpack tex from (-1,1) coords
	// Runtime:
	_refcount?: i32; // Number of users
	_handle?: string; // Handle used to retrieve this object in Data
	_vertexBuffer?: VertexBufferRaw;
	_vertexBufferMap?: Map<string, VertexBufferRaw>;
	_indexBuffers?: IndexBufferRaw[];
	_ready?: bool;
	_vertices?: DataView;
	_indices?: Uint32Array[];
	_materialIndices?: i32[];
	_struct?: VertexStructureRaw;
	_instancedVB?: VertexBufferRaw;
	_instanced?: bool;
	_instanceCount?: i32;
	///if arm_skin
	_skeletonTransformsI?: TMat4[];
	_actions?: Map<string, TObj[]>;
	_mats?: Map<string, TMat4[]>;
	///end
}

type TSkin = {
	transform: TTransform;
	bone_ref_array: string[];
	bone_len_array: Float32Array;
	transformsI: Float32Array[]; // per-bone, size = 16, with skin.transform, pre-inverted
	bone_count_array: Int16Array;
	bone_index_array: Int16Array;
	bone_weight_array: Int16Array;
}

type TVertexArray = {
	attrib: string;
	values: Int16Array;
	data: string; // short4norm, short2norm
	padding?: Null<i32>;
	// Runtime:
	_size?: Null<i32>;
}

type TIndexArray = {
	values: Uint32Array; // size = 3
	material: i32;
}

type TLightData = {
	name: string;
	type: string; // sun, point, spot
	color: Float32Array;
	strength: f32;
	near_plane?: Null<f32>;
	far_plane?: Null<f32>;
	fov?: Null<f32>;
	size?: Null<f32>; // Area light
	size_y?: Null<f32>;
}

type TCameraData = {
	name: string;
	near_plane: f32;
	far_plane: f32;
	fov: f32;
	clear_color?: Float32Array;
	aspect?: Null<f32>;
	frustum_culling?: Null<bool>;
	ortho?: Float32Array; // Indicates ortho camera, left, right, bottom, top
}

type TMaterialData = {
	name: string;
	shader: string;
	contexts: TMaterialContext[];
	skip_context?: string;
	override_context?: TShaderOverride;
	// Runtime:
	_uid: f32;
	_shader: TShaderData;
	_contexts: TMaterialContext[];
}

type TShaderOverride = {
	cull_mode?: string;
	addressing?: string;
	filter?: string;
	shared_sampler?: string;
}

type TMaterialContext = {
	name: string;
	bind_constants?: TBindConstant[];
	bind_textures?: TBindTexture[];
	// Runtime:
	_textures?: ImageRaw[];
}

type TBindConstant = {
	name: string;
	vec4?: Float32Array;
	vec3?: Float32Array;
	vec2?: Float32Array;
	float?: Null<f32>;
	bool?: Null<bool>;
	int?: Null<i32>;
}

type TBindTexture = {
	name: string;
	file: string;
	format?: string; // RGBA32, RGBA64, R8
	generate_mipmaps?: Null<bool>;
	mipmaps?: string[]; // Reference image names
	u_addressing?: string;
	v_addressing?: string;
	min_filter?: string;
	mag_filter?: string;
	mipmap_filter?: string;
	source?: string; // file, movie
}

type TShaderData = {
	name: string;
	contexts: TShaderContext[];
	// Runtime:
	_contexts: TShaderContext[];
}

type TShaderContext = {
	name: string;
	depth_write: bool;
	compare_mode: string;
	cull_mode: string;
	vertex_elements: TVertexElement[];
	vertex_shader: string;
	fragment_shader: string;
	geometry_shader?: string;
	constants?: TShaderConstant[];
	texture_units?: TTextureUnit[];
	blend_source?: string;
	blend_destination?: string;
	blend_operation?: string;
	alpha_blend_source?: string;
	alpha_blend_destination?: string;
	alpha_blend_operation?: string;
	color_writes_red?: bool[]; // Per target masks
	color_writes_green?: bool[];
	color_writes_blue?: bool[];
	color_writes_alpha?: bool[];
	color_attachments?: string[]; // RGBA32, RGBA64, R8
	depth_attachment?: string; // DEPTH32
	shader_from_source?: Null<bool>; // Build shader at runtime using fromSource()
	// Runtime:
	_pipeState?: PipelineStateRaw;
	_constants?: ConstantLocation[];
	_textureUnits?: TextureUnit[];
	_overrideContext?: TShaderOverride;
	_structure?: VertexStructureRaw;
	_instancingType?: i32;
}

type TVertexElement = {
	name: string;
	data: string; // "short4norm", "short2norm"
}

type TShaderConstant = {
	name: string;
	type: string;
	link?: string;
	vec4?: Float32Array;
	vec3?: Float32Array;
	vec2?: Float32Array;
	float?: Null<f32>;
	bool?: Null<bool>;
	int?: Null<i32>;
}

type TTextureUnit = {
	name: string;
	is_image?: Null<bool>; // image2D
	link?: string;
	addressing_u?: string;
	addressing_v?: string;
	filter_min?: string;
	filter_mag?: string;
	mipmap_filter?: string;
}

type TSpeakerData = {
	name: string;
	sound: string;
	muted: bool;
	loop: bool;
	stream: bool;
	volume: f32;
	attenuation: f32;
	play_on_start: bool;
}

type TWorldData = {
	name: string;
	background_color: i32;
	strength: f32;
	irradiance?: string; // Reference to TIrradiance blob
	radiance?: string;
	radiance_mipmaps?: Null<i32>;
	envmap?: string;
	// Runtime:
	_envmap?: ImageRaw;
	_radiance?: ImageRaw;
	_radianceMipmaps?: ImageRaw[];
	_irradiance?: Float32Array;
}

type TIrradiance = {
	irradiance: Float32Array; // Blob with spherical harmonics, bands 0,1,2
}

type TParticleData = {
	name: string;
	type: i32; // 0 - Emitter, Hair
	loop: bool;
	count: i32;
	frame_start: f32;
	frame_end: f32;
	lifetime: f32;
	lifetime_random: f32;
	emit_from: i32; // 0 - Vert, 1 - Face, 2 - Volume
	object_align_factor: Float32Array;
	factor_random: f32;
	physics_type: i32; // 0 - No, 1 - Newton
	particle_size: f32; // Object scale
	size_random: f32; // Random scale
	mass: f32;
	instance_object: string; // Object reference
	weight_gravity: f32;
}

type TParticleReference = {
	name: string;
	particle: string;
	seed: i32;
}

type TObj = {
	type: string; // object, mesh_object, light_object, camera_object, speaker_object, decal_object
	name: string;
	data_ref: string;
	transform: TTransform;
	material_refs?: string[];
	particle_refs?: TParticleReference[];
	render_emitter?: bool;
	is_particle?: Null<bool>; // This object is used as a particle object
	children?: TObj[];
	dimensions?: Float32Array; // Geometry objects
	object_actions?: string[];
	bone_actions?: string[];
	anim?: TAnimation; // Bone/object animation
	parent?: TObj;
	parent_bone?: string;
	parent_bone_tail?: Float32Array; // Translate from head to tail
	parent_bone_tail_pose?: Float32Array;
	parent_bone_connected?: Null<bool>;
	visible?: Null<bool>;
	spawn?: Null<bool>; // Auto add object when creating scene
	local_only?: Null<bool>; // Apply parent matrix
	sampled?: Null<bool>; // Object action
	is_ik_fk_only?: Null<bool>; // Bone IK or FK only
}

type TTransform = {
	target?: string;
	values: Float32Array;
}

type TAnimation = {
	tracks: TTrack[];
	begin?: Null<i32>; // Frames, for non-sampled
	end?: Null<i32>;
	has_delta?: Null<bool>; // Delta transform
	marker_frames?: Uint32Array;
	marker_names?: string[];
}

type TTrack = {
	target: string;
	frames: Uint32Array;
	values: Float32Array; // sampled - full matrix transforms, non-sampled - values
}
