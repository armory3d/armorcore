
type scene_t = {
	name?: string;
	mesh_datas?: mesh_data_t[];
	light_datas?: light_data_t[];
	camera_datas?: camera_data_t[];
	camera_ref?: string; // Active camera
	material_datas?: material_data_t[];
	particle_datas?: particle_data_t[];
	shader_datas?: shader_data_t[];
	speaker_datas?: speaker_data_t[];
	world_datas?: world_data_t[];
	world_ref?: string;
	objects?: obj_t[];
	embedded_datas?: string[]; // Preload for this scene, images only for now
}

type mesh_data_t = {
	name: string;
	vertex_arrays: vertex_array_t[];
	index_arrays: index_array_t[];
	skin?: skin_t;
	instanced_data?: Float32Array;
	instanced_type?: Null<i32>; // off, loc, loc+rot, loc+scale, loc+rot+scale
	scale_pos?: Null<f32>; // Unpack pos from (-1,1) coords
	scale_tex?: Null<f32>; // Unpack tex from (-1,1) coords
	// Runtime:
	_refcount?: i32; // Number of users
	_handle?: string; // Handle used to retrieve this object in Data
	_vertex_buffer?: vertex_buffer_t;
	_vertex_buffer_map?: Map<string, vertex_buffer_t>;
	_index_buffers?: index_buffer_t[];
	_ready?: bool;
	_vertices?: DataView;
	_indices?: Uint32Array[];
	_material_indices?: i32[];
	_struct?: vertex_struct_t;
	_instanced_vb?: vertex_buffer_t;
	_instanced?: bool;
	_instance_count?: i32;
	///if arm_skin
	_skeleton_transforms_inv?: mat4_t[];
	_actions?: Map<string, obj_t[]>;
	_mats?: Map<string, mat4_t[]>;
	///end
}

type skin_t = {
	transform: transform_values_t;
	bone_ref_array: string[];
	bone_len_array: Float32Array;
	transforms_inv: Float32Array[]; // per-bone, size = 16, with skin.transform, pre-inverted
	bone_count_array: Int16Array;
	bone_index_array: Int16Array;
	bone_weight_array: Int16Array;
}

type vertex_array_t = {
	attrib: string;
	values: Int16Array;
	data: string; // short4norm, short2norm
	padding?: Null<i32>;
	// Runtime:
	_size?: Null<i32>;
}

type index_array_t = {
	values: Uint32Array; // size = 3
	material: i32;
}

type light_data_t = {
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

type camera_data_t = {
	name: string;
	near_plane: f32;
	far_plane: f32;
	fov: f32;
	clear_color?: Float32Array;
	aspect?: Null<f32>;
	frustum_culling?: Null<bool>;
	ortho?: Float32Array; // Indicates ortho camera, left, right, bottom, top
}

type material_data_t = {
	name: string;
	shader: string;
	contexts: material_context_t[];
	skip_context?: string;
	override_context?: shader_override_t;
	// Runtime:
	_uid?: f32;
	_shader?: shader_data_t;
	_contexts?: material_context_t[];
}

type shader_override_t = {
	cull_mode?: string;
	addressing?: string;
	filter?: string;
	shared_sampler?: string;
}

type material_context_t = {
	name: string;
	bind_constants?: bind_const_t[];
	bind_textures?: bind_tex_t[];
	// Runtime:
	_textures?: image_t[];
}

type bind_const_t = {
	name: string;
	vec4?: Float32Array;
	vec3?: Float32Array;
	vec2?: Float32Array;
	float?: Null<f32>;
	bool?: Null<bool>;
	int?: Null<i32>;
}

type bind_tex_t = {
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

type shader_data_t = {
	name: string;
	contexts: shader_context_t[];
	// Runtime:
	_contexts?: shader_context_t[];
}

type shader_context_t = {
	name: string;
	depth_write: bool;
	compare_mode: string;
	cull_mode: string;
	vertex_elements: vertex_element_t[];
	vertex_shader: string;
	fragment_shader: string;
	geometry_shader?: string;
	constants?: shader_const_t[];
	texture_units?: tex_unit_t[];
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
	shader_from_source?: Null<bool>; // Build shader at runtime using from_source()
	// Runtime:
	_pipe_state?: pipeline_t;
	_constants?: kinc_const_loc_t[];
	_tex_units?: kinc_tex_unit_t[];
	_override_context?: shader_override_t;
	_structure?: vertex_struct_t;
	_instancing_type?: i32;
}

type vertex_element_t = {
	name: string;
	data: string; // "short4norm", "short2norm"
}

type shader_const_t = {
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

type tex_unit_t = {
	name: string;
	is_image?: Null<bool>; // image2D
	link?: string;
	addressing_u?: string;
	addressing_v?: string;
	filter_min?: string;
	filter_mag?: string;
	mipmap_filter?: string;
}

type speaker_data_t = {
	name: string;
	sound: string;
	muted: bool;
	loop: bool;
	stream: bool;
	volume: f32;
	attenuation: f32;
	play_on_start: bool;
}

type world_data_t = {
	name: string;
	background_color: i32;
	strength: f32;
	irradiance?: string; // Reference to TIrradiance blob
	radiance?: string;
	radiance_mipmaps?: Null<i32>;
	envmap?: string;
	// Runtime:
	_envmap?: image_t;
	_radiance?: image_t;
	_radiance_mipmaps?: image_t[];
	_irradiance?: Float32Array;
}

type irradiance_t = {
	irradiance: Float32Array; // Blob with spherical harmonics, bands 0,1,2
}

type particle_data_t = {
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

type particle_ref_t = {
	name: string;
	particle: string;
	seed: i32;
}

type obj_t = {
	type: string; // object, mesh_object, light_object, camera_object, speaker_object, decal_object
	name: string;
	data_ref: string;
	transform: transform_values_t;
	material_refs?: string[];
	particle_refs?: particle_ref_t[];
	render_emitter?: bool;
	is_particle?: Null<bool>; // This object is used as a particle object
	children?: obj_t[];
	dimensions?: Float32Array; // Geometry objects
	object_actions?: string[];
	bone_actions?: string[];
	anim?: anim_t; // Bone/object animation
	parent?: obj_t;
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

type transform_values_t = {
	target?: string;
	values: Float32Array;
}

type anim_t = {
	tracks: track_t[];
	begin?: Null<i32>; // Frames, for non-sampled
	end?: Null<i32>;
	has_delta?: Null<bool>; // Delta transform
	marker_frames?: Uint32Array;
	marker_names?: string[];
}

type track_t = {
	target: string;
	frames: Uint32Array;
	values: Float32Array; // sampled - full matrix transforms, non-sampled - values
}
