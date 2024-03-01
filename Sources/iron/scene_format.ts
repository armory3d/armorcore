
type scene_t = {
	name?: string;
	objects?: obj_t[];
	mesh_datas?: mesh_data_t[];
	light_datas?: light_data_t[];
	camera_datas?: camera_data_t[];
	camera_ref?: string;
	material_datas?: material_data_t[];
	shader_datas?: shader_data_t[];
	world_datas?: world_data_t[];
	world_ref?: string;
	particle_datas?: particle_data_t[];
	speaker_datas?: speaker_data_t[];
	embedded_datas?: string[]; // Preload for this scene, images only for now
};

type mesh_data_t = {
	name?: string;
	scale_pos?: f32; // Unpack pos from (-1,1) coords
	scale_tex?: f32; // Unpack tex from (-1,1) coords
	instancing?: mesh_data_instancing_t;
	skin?: skin_t;
	vertex_arrays?: vertex_array_t[];
	index_arrays?: index_array_t[];
	_?: mesh_data_runtime_t;
};

type mesh_data_runtime_t = {
	refcount?: i32; // Number of users
	handle?: string; // Handle used to retrieve this object in Data
	vertex_buffer?: vertex_buffer_t;
	vertex_buffer_map?: map_t<string, vertex_buffer_t>;
	index_buffers?: index_buffer_t[];
	ready?: bool;
	vertices?: buffer_view_t;
	indices?: u32_array_t[];
	material_indices?: i32[];
	structure?: vertex_struct_t;
	instanced_vb?: vertex_buffer_t;
	instanced?: bool;
	instance_count?: i32;
	///if arm_skin
	skeleton_transforms_inv?: mat4_t[];
	actions?: map_t<string, obj_t[]>;
	mats?: map_t<string, mat4_t[]>;
	///end
};

type mesh_data_instancing_t = {
	type?: i32; // off, loc, loc+rot, loc+scale, loc+rot+scale
	data?: f32_array_t;
};

type skin_t = {
	transform?: f32_array_t;
	bone_ref_array?: string[];
	bone_len_array?: f32_array_t;
	transforms_inv?: f32_array_t[]; // per-bone, size = 16, with skin.transform, pre-inverted
	bone_count_array?: i16_array_t;
	bone_index_array?: i16_array_t;
	bone_weight_array?: i16_array_t;
};

type vertex_array_t = {
	attrib?: string;
	data?: string; // short4norm, short2norm
	values?: i16_array_t;
};

type index_array_t = {
	material?: i32;
	values?: u32_array_t; // size = 3
};

type light_data_t = {
	name?: string;
	color?: i32;
	strength?: f32;
	size?: f32; // Area light
	size_y?: f32;
};

type camera_data_t = {
	name?: string;
	near_plane?: f32;
	far_plane?: f32;
	fov?: f32;
	aspect?: f32;
	frustum_culling?: bool;
	ortho?: f32_array_t; // Indicates ortho camera, left, right, bottom, top
};

type material_data_t = {
	name?: string;
	shader?: string;
	contexts?: material_context_t[];
	_?: material_data_runtime_t;
};

type material_data_runtime_t = {
	uid?: f32;
	shader?: shader_data_t;
	contexts?: material_context_t[];
};

type material_context_t = {
	name?: string;
	bind_constants?: bind_const_t[];
	bind_textures?: bind_tex_t[];
	_?: material_context_runtime_t;
};

type material_context_runtime_t = {
	textures?: image_t[];
};

type bind_const_t = {
	name?: string;
	vec?: f32_array_t; // bool (vec[0] > 0) | i32 | f32 | vec2 | vec3 | vec4
};

type bind_tex_t = {
	name?: string;
	file?: string;
	u_addressing?: string;
	v_addressing?: string;
	min_filter?: string;
	mag_filter?: string;
	mipmap_filter?: string;
	generate_mipmaps?: bool;
	mipmaps?: string[]; // Reference image names
};

type shader_data_t = {
	name?: string;
	contexts?: shader_context_t[];
	_?: shader_data_runtime_t;
};

type shader_data_runtime_t = {
	contexts?: shader_context_t[];
};

type shader_context_t = {
	name?: string;
	depth_write?: bool;
	compare_mode?: string;
	cull_mode?: string;
	vertex_shader?: string;
	fragment_shader?: string;
	geometry_shader?: string;
	shader_from_source?: bool; // Build shader at runtime using from_source()
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
	vertex_elements?: vertex_element_t[];
	constants?: shader_const_t[];
	texture_units?: tex_unit_t[];
	_?: shader_context_runtime_t;
};

type shader_context_runtime_t = {
	pipe_state?: pipeline_t;
	constants?: kinc_const_loc_t[];
	tex_units?: kinc_tex_unit_t[];
	override_context?: _shader_override_t;
	structure?: vertex_struct_t;
	instancing_type?: i32;
};

type _shader_override_t = {
	addressing?: string;
	filter?: string;
	shared_sampler?: string;
};

type vertex_element_t = {
	name?: string;
	data?: string; // "short4norm", "short2norm"
};

type shader_const_t = {
	name?: string;
	type?: string;
	link?: string;
};

type tex_unit_t = {
	name?: string;
	link?: string;
	image_uniform?: bool; // uniform layout(r8) writeonly image3D voxels
};

type speaker_data_t = {
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
	name?: string;
	color?: i32;
	strength?: f32;
	irradiance?: string; // Reference to irradiance_t blob
	radiance?: string;
	radiance_mipmaps?: i32;
	envmap?: string;
	_?: world_data_runtime_t;
};

type world_data_runtime_t = {
	envmap?: image_t;
	radiance?: image_t;
	radiance_mipmaps?: image_t[];
	irradiance?: f32_array_t;
};

type irradiance_t = {
	irradiance?: f32_array_t; // Blob with spherical harmonics, bands 0,1,2
};

type particle_data_t = {
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
	name?: string;
	particle?: string;
	seed?: i32;
};

type obj_t = {
	name?: string;
	type?: string; // object, mesh_object, light_object, camera_object, speaker_object, decal_object
	data_ref?: string;
	transform?: f32_array_t;
	dimensions?: f32_array_t; // Geometry objects
	visible?: bool;
	spawn?: bool; // Auto add object when creating scene
	particles?: particles_t;
	anim?: anim_t; // Bone/object animation
	material_refs?: string[];
	children?: obj_t[];
};

type particles_t = {
	refs?: particle_ref_t[];
	render_emitter?: bool;
	is_particle?: bool; // This object is used as a particle object
};

type anim_t = {
	object_actions?: string[];
	bone_actions?: string[];
	parent_bone?: string;
	parent_bone_tail?: f32_array_t; // Translate from head to tail
	parent_bone_tail_pose?: f32_array_t;
	parent_bone_connected?: bool;
	tracks?: track_t[];
	begin?: i32; // Frames, for non-sampled
	end?: i32;
	has_delta?: bool; // Delta transform
	marker_frames?: u32_array_t;
	marker_names?: string[];
};

type track_t = {
	target?: string;
	frames?: u32_array_t;
	values?: f32_array_t; // sampled - full matrix transforms, non-sampled - values
};
