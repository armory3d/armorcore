
type scene_t = {
	// Opt
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
};

type mesh_data_t = {
	// Base
	name?: string;
	vertex_arrays?: vertex_array_t[];
	index_arrays?: index_array_t[];
	// Opt
	skin?: skin_t;
	instanced_data?: f32_array_t;
	instanced_type?: i32; // off, loc, loc+rot, loc+scale, loc+rot+scale
	scale_pos?: f32; // Unpack pos from (-1,1) coords
	scale_tex?: f32; // Unpack tex from (-1,1) coords
	// Runtime:
	_refcount?: i32; // Number of users
	_handle?: string; // Handle used to retrieve this object in Data
	_vertex_buffer?: vertex_buffer_t;
	_vertex_buffer_map?: map_t<string, vertex_buffer_t>;
	_index_buffers?: index_buffer_t[];
	_ready?: bool;
	_vertices?: buffer_view_t;
	_indices?: u32_array_t[];
	_material_indices?: i32[];
	_struct?: vertex_struct_t;
	_instanced_vb?: vertex_buffer_t;
	_instanced?: bool;
	_instance_count?: i32;
	///if arm_skin
	_skeleton_transforms_inv?: mat4_t[];
	_actions?: map_t<string, obj_t[]>;
	_mats?: map_t<string, mat4_t[]>;
	///end
};

type skin_t = {
	// Base
	transform?: transform_values_t;
	bone_ref_array?: string[];
	bone_len_array?: f32_array_t;
	transforms_inv?: f32_array_t[]; // per-bone, size = 16, with skin.transform, pre-inverted
	bone_count_array?: i16_array_t;
	bone_index_array?: i16_array_t;
	bone_weight_array?: i16_array_t;
};

type vertex_array_t = {
	// Base
	attrib?: string;
	values?: i16_array_t;
	data?: string; // short4norm, short2norm
	// Opt
	padding?: i32;
	// Runtime:
	_size?: i32;
};

type index_array_t = {
	// Base
	values?: u32_array_t; // size = 3
	material?: i32;
};

type light_data_t = {
	// Base
	name?: string;
	type?: string; // sun, point, spot
	color?: f32_array_t;
	strength?: f32;
	// Opt
	near_plane?: f32;
	far_plane?: f32;
	fov?: f32;
	size?: f32; // Area light
	size_y?: f32;
};

type camera_data_t = {
	// Base
	name?: string;
	near_plane?: f32;
	far_plane?: f32;
	fov?: f32;
	// Opt
	clear_color?: f32_array_t;
	aspect?: f32;
	frustum_culling?: bool;
	ortho?: f32_array_t; // Indicates ortho camera, left, right, bottom, top
};

type material_data_t = {
	// Base
	name?: string;
	shader?: string;
	contexts?: material_context_t[];
	// Opt
	skip_context?: string;
	override_context?: shader_override_t;
	// Runtime:
	_uid?: f32;
	_shader?: shader_data_t;
	_contexts?: material_context_t[];
};

type shader_override_t = {
	// Opt
	cull_mode?: string;
	addressing?: string;
	filter?: string;
	shared_sampler?: string;
};

type material_context_t = {
	// Base
	name?: string;
	// Opt
	bind_constants?: bind_const_t[];
	bind_textures?: bind_tex_t[];
	// Runtime:
	_textures?: image_t[];
};

type bind_const_t = {
	// Base
	name?: string;
	// Opt
	vec4?: f32_array_t;
	vec3?: f32_array_t;
	vec2?: f32_array_t;
	vec1?: f32; // bool (vec1 > 0) | i32 | f32
};

type bind_tex_t = {
	// Base
	name?: string;
	file?: string;
	// Opt
	format?: string; // RGBA32, RGBA64, R8
	generate_mipmaps?: bool;
	mipmaps?: string[]; // Reference image names
	u_addressing?: string;
	v_addressing?: string;
	min_filter?: string;
	mag_filter?: string;
	mipmap_filter?: string;
	source?: string; // file, movie
};

type shader_data_t = {
	// Base
	name?: string;
	contexts?: shader_context_t[];
	// Runtime:
	_contexts?: shader_context_t[];
};

type shader_context_t = {
	// Base
	name?: string;
	depth_write?: bool;
	compare_mode?: string;
	cull_mode?: string;
	vertex_elements?: vertex_element_t[];
	vertex_shader?: string;
	fragment_shader?: string;
	// Opt
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
	shader_from_source?: bool; // Build shader at runtime using from_source()
	// Runtime:
	_pipe_state?: pipeline_t;
	_constants?: kinc_const_loc_t[];
	_tex_units?: kinc_tex_unit_t[];
	_override_context?: shader_override_t;
	_structure?: vertex_struct_t;
	_instancing_type?: i32;
};

type vertex_element_t = {
	// Base
	name?: string;
	data?: string; // "short4norm", "short2norm"
};

type shader_const_t = {
	// Base
	name?: string;
	type?: string;
	// Opt
	link?: string;
	vec4?: f32_array_t;
	vec3?: f32_array_t;
	vec2?: f32_array_t;
	vec1?: f32; // bool (vec1 > 0) | i32 | f32
};

type tex_unit_t = {
	// Base
	name?: string;
	// Opt
	is_image?: bool; // image2D
	link?: string;
	addressing_u?: string;
	addressing_v?: string;
	filter_min?: string;
	filter_mag?: string;
	mipmap_filter?: string;
};

type speaker_data_t = {
	// Base
	name?: string;
	sound?: string;
	muted?: bool;
	loop?: bool;
	stream?: bool;
	volume?: f32;
	attenuation?: f32;
	play_on_start?: bool;
};

type world_data_t = {
	// Base
	name?: string;
	background_color?: i32;
	strength?: f32;
	// Opt
	irradiance?: string; // Reference to irradiance_t blob
	radiance?: string;
	radiance_mipmaps?: i32;
	envmap?: string;
	// Runtime:
	_envmap?: image_t;
	_radiance?: image_t;
	_radiance_mipmaps?: image_t[];
	_irradiance?: f32_array_t;
};

type irradiance_t = {
	// Base
	irradiance?: f32_array_t; // Blob with spherical harmonics, bands 0,1,2
};

type particle_data_t = {
	// Base
	name?: string;
	type?: i32; // 0 - Emitter, Hair
	loop?: bool;
	count?: i32;
	frame_start?: f32;
	frame_end?: f32;
	lifetime?: f32;
	lifetime_random?: f32;
	emit_from?: i32; // 0 - Vert, 1 - Face, 2 - Volume
	object_align_factor?: f32_array_t;
	factor_random?: f32;
	physics_type?: i32; // 0 - No, 1 - Newton
	particle_size?: f32; // Object scale
	size_random?: f32; // Random scale
	mass?: f32;
	instance_object?: string; // Object reference
	weight_gravity?: f32;
};

type particle_ref_t = {
	// Base
	name?: string;
	particle?: string;
	seed?: i32;
};

type obj_t = {
	// Base
	type?: string; // object, mesh_object, light_object, camera_object, speaker_object, decal_object
	name?: string;
	data_ref?: string;
	transform?: transform_values_t;
	// Opt
	material_refs?: string[];
	particle_refs?: particle_ref_t[];
	render_emitter?: bool;
	is_particle?: bool; // This object is used as a particle object
	children?: obj_t[];
	dimensions?: f32_array_t; // Geometry objects
	object_actions?: string[];
	bone_actions?: string[];
	anim?: anim_t; // Bone/object animation
	parent?: obj_t;
	parent_bone?: string;
	parent_bone_tail?: f32_array_t; // Translate from head to tail
	parent_bone_tail_pose?: f32_array_t;
	parent_bone_connected?: bool;
	visible?: bool;
	spawn?: bool; // Auto add object when creating scene
	local_only?: bool; // Apply parent matrix
	sampled?: bool; // Object action
	is_ik_fk_only?: bool; // Bone IK or FK only
};

type transform_values_t = {
	// Base
	values?: f32_array_t;
	// Opt
	target?: string;
};

type anim_t = {
	// Base
	tracks?: track_t[];
	// Opt
	begin?: i32; // Frames, for non-sampled
	end?: i32;
	has_delta?: bool; // Delta transform
	marker_frames?: u32_array_t;
	marker_names?: string[];
};

type track_t = {
	// Base
	target?: string;
	frames?: u32_array_t;
	values?: f32_array_t; // sampled - full matrix transforms, non-sampled - values
};
