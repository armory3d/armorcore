package iron.data;

import js.lib.Float32Array;
import js.lib.Uint32Array;
import js.lib.Int16Array;

typedef TSceneFormat = {
	@:optional public var name: String;
	@:optional public var mesh_datas: Array<TMeshData>;
	@:optional public var light_datas: Array<TLightData>;
	@:optional public var camera_datas: Array<TCameraData>;
	@:optional public var camera_ref: String; // Active camera
	@:optional public var material_datas: Array<TMaterialData>;
	@:optional public var particle_datas: Array<TParticleData>;
	@:optional public var shader_datas: Array<TShaderData>;
	@:optional public var speaker_datas: Array<TSpeakerData>;
	@:optional public var world_datas: Array<TWorldData>;
	@:optional public var world_ref: String;
	@:optional public var objects: Array<TObj>;
	@:optional public var embedded_datas: Array<String>; // Preload for this scene, images only for now
	@:optional public var irradiance: Float32Array; // Blob with spherical harmonics, bands 0,1,2
}

typedef TMeshData = {
	public var name: String;
	public var vertex_arrays: Array<TVertexArray>;
	public var index_arrays: Array<TIndexArray>;
	@:optional public var skin: TSkin;
	@:optional public var instanced_data: Float32Array;
	@:optional public var instanced_type: Null<Int>; // off, loc, loc+rot, loc+scale, loc+rot+scale
	@:optional public var scale_pos: Null<Float>; // Unpack pos from (-1,1) coords
	@:optional public var scale_tex: Null<Float>; // Unpack tex from (-1,1) coords
}

typedef TSkin = {
	public var transform: TTransform;
	public var bone_ref_array: Array<String>;
	public var bone_len_array: Float32Array;
	public var transformsI: Array<Float32Array>; // per-bone, size = 16, with skin.transform, pre-inverted
	public var bone_count_array: Int16Array;
	public var bone_index_array: Int16Array;
	public var bone_weight_array: Int16Array;
	public var constraints: Array<TConstraint>;
}

typedef TVertexArray = {
	public var attrib: String;
	public var values: Int16Array;
	public var data: String; // short4norm, short2norm
	@:optional public var padding: Null<Int>;
	@:optional public var size: Null<Int>;
}

typedef TIndexArray = {
	public var values: Uint32Array; // size = 3
	public var material: Int;
}

typedef TLightData = {
	public var name: String;
	public var type: String; // sun, point, spot
	public var color: Float32Array;
	public var strength: Float;
	@:optional public var near_plane: Null<Float>;
	@:optional public var far_plane: Null<Float>;
	@:optional public var fov: Null<Float>;
	@:optional public var size: Null<Float>; // Area light
	@:optional public var size_y: Null<Float>;
}

typedef TCameraData = {
	public var name: String;
	public var near_plane: Float;
	public var far_plane: Float;
	public var fov: Float;
	@:optional public var clear_color: Float32Array;
	@:optional public var aspect: Null<Float>;
	@:optional public var frustum_culling: Null<Bool>;
	@:optional public var ortho: Float32Array; // Indicates ortho camera, left, right, bottom, top
}

typedef TMaterialData = {
	public var name: String;
	public var shader: String;
	public var contexts: Array<TMaterialContext>;
	@:optional public var skip_context: String;
	@:optional public var override_context: TShaderOverride;
}

typedef TShaderOverride = {
	@:optional public var cull_mode: String;
	@:optional public var addressing: String;
	@:optional public var filter: String;
	@:optional public var shared_sampler: String;
}

typedef TMaterialContext = {
	public var name: String;
	@:optional public var bind_constants: Array<TBindConstant>;
	@:optional public var bind_textures: Array<TBindTexture>;
}

typedef TBindConstant = {
	public var name: String;
	@:optional public var vec4: Float32Array;
	@:optional public var vec3: Float32Array;
	@:optional public var vec2: Float32Array;
	@:optional public var float: Null<Float>;
	@:optional public var bool: Null<Bool>;
	@:optional public var int: Null<Int>;
}

typedef TBindTexture = {
	public var name: String;
	public var file: String;
	@:optional public var format: String; // RGBA32, RGBA64, R8
	@:optional public var generate_mipmaps: Null<Bool>;
	@:optional public var mipmaps: Array<String>; // Reference image names
	@:optional public var u_addressing: String;
	@:optional public var v_addressing: String;
	@:optional public var min_filter: String;
	@:optional public var mag_filter: String;
	@:optional public var mipmap_filter: String;
	@:optional public var source: String; // file, movie
}

typedef TShaderData = {
	public var name: String;
	public var contexts: Array<TShaderContext>;
}

typedef TShaderContext = {
	public var name: String;
	public var depth_write: Bool;
	public var compare_mode: String;
	public var cull_mode: String;
	public var vertex_elements: Array<TVertexElement>;
	public var vertex_shader: String;
	public var fragment_shader: String;
	@:optional public var geometry_shader: String;
	@:optional public var constants: Array<TShaderConstant>;
	@:optional public var texture_units: Array<TTextureUnit>;
	@:optional public var blend_source: String;
	@:optional public var blend_destination: String;
	@:optional public var blend_operation: String;
	@:optional public var alpha_blend_source: String;
	@:optional public var alpha_blend_destination: String;
	@:optional public var alpha_blend_operation: String;
	@:optional public var color_writes_red: Array<Bool>; // Per target masks
	@:optional public var color_writes_green: Array<Bool>;
	@:optional public var color_writes_blue: Array<Bool>;
	@:optional public var color_writes_alpha: Array<Bool>;
	@:optional public var color_attachments: Array<String>; // RGBA32, RGBA64, R8
	@:optional public var depth_attachment: String; // DEPTH32
	@:optional public var conservative_raster: Null<Bool>;
	@:optional public var shader_from_source: Null<Bool>; // Build shader at runtime using fromSource()
}

typedef TVertexElement = {
	public var name: String;
	public var data: String; // "short4norm", "short2norm"
}

typedef TShaderConstant = {
	public var name: String;
	public var type: String;
	@:optional public var link: String;
	@:optional public var vec4: Float32Array;
	@:optional public var vec3: Float32Array;
	@:optional public var vec2: Float32Array;
	@:optional public var float: Null<Float>;
	@:optional public var bool: Null<Bool>;
	@:optional public var int: Null<Int>;
}

typedef TTextureUnit = {
	public var name: String;
	@:optional public var is_image: Null<Bool>; // image2D
	@:optional public var link: String;
	@:optional public var addressing_u: String;
	@:optional public var addressing_v: String;
	@:optional public var filter_min: String;
	@:optional public var filter_mag: String;
	@:optional public var mipmap_filter: String;
}

typedef TSpeakerData = {
	public var name: String;
	public var sound: String;
	public var muted: Bool;
	public var loop: Bool;
	public var stream: Bool;
	public var volume: Float;
	public var pitch: Float;
	public var volume_min: Float;
	public var volume_max: Float;
	public var attenuation: Float;
	public var distance_max: Float;
	public var distance_reference: Float;
	public var play_on_start: Bool;
}

typedef TWorldData = {
	public var name: String;
	public var background_color: Int;
	public var probe: TProbeData;
	@:optional public var envmap: String;
}

typedef TParticleData = {
	public var name: String;
	public var type: Int; // 0 - Emitter, Hair
	public var loop: Bool;
	public var count: Int;
	public var frame_start: Float;
	public var frame_end: Float;
	public var lifetime: Float;
	public var lifetime_random: Float;
	public var emit_from: Int; // 0 - Vert, 1 - Face, 2 - Volume
	public var object_align_factor: Float32Array;
	public var factor_random: Float;
	public var physics_type: Int; // 0 - No, 1 - Newton
	public var particle_size: Float; // Object scale
	public var size_random: Float; // Random scale
	public var mass: Float;
	public var instance_object: String; // Object reference
	public var weight_gravity: Float;
}

typedef TParticleReference = {
	public var name: String;
	public var particle: String;
	public var seed: Int;
}

typedef TObj = {
	public var type: String; // object, mesh_object, light_object, camera_object, speaker_object, decal_object
	public var name: String;
	public var data_ref: String;
	public var transform: TTransform;
	@:optional public var material_refs: Array<String>;
	@:optional public var particle_refs: Array<TParticleReference>;
	@:optional public var render_emitter: Bool;
	@:optional public var is_particle: Null<Bool>; // This object is used as a particle object
	@:optional public var children: Array<TObj>;
	@:optional public var dimensions: Float32Array; // Geometry objects
	@:optional public var object_actions: Array<String>;
	@:optional public var bone_actions: Array<String>;
	@:optional public var anim: TAnimation; // Bone/object animation
	@:optional public var parent: TObj;
	@:optional public var parent_bone: String;
	@:optional public var parent_bone_tail: Float32Array; // Translate from head to tail
	@:optional public var parent_bone_tail_pose: Float32Array;
	@:optional public var parent_bone_connected: Null<Bool>;
	@:optional public var visible: Null<Bool>;
	@:optional public var visible_mesh: Null<Bool>;
	@:optional public var mobile: Null<Bool>;
	@:optional public var spawn: Null<Bool>; // Auto add object when creating scene
	@:optional public var local_only: Null<Bool>; // Apply parent matrix
	@:optional public var sampled: Null<Bool>; // Object action
	@:optional public var is_ik_fk_only: Null<Bool>; // Bone IK or FK only
	@:optional public var relative_bone_constraints: Null<Bool>; // Use parent relative bone constraints
}

typedef TConstraint = {
	public var name: String;
	public var type: String;
	@:optional public var bone: String; // Bone constraint
	@:optional public var target: String;
	@:optional public var use_x: Null<Bool>;
	@:optional public var use_y: Null<Bool>;
	@:optional public var use_z: Null<Bool>;
	@:optional public var invert_x: Null<Bool>;
	@:optional public var invert_y: Null<Bool>;
	@:optional public var invert_z: Null<Bool>;
	@:optional public var use_offset: Null<Bool>;
	@:optional public var influence: Null<Float>;
}

typedef TTransform = {
	@:optional public var target: String;
	public var values: Float32Array;
}

typedef TAnimation = {
	public var tracks: Array<TTrack>;
	@:optional public var begin: Null<Int>; // Frames, for non-sampled
	@:optional public var end: Null<Int>;
	@:optional public var has_delta: Null<Bool>; // Delta transform
	@:optional public var marker_frames: Uint32Array;
	@:optional public var marker_names: Array<String>;
}

typedef TAnimationTransform = {
	public var type: String; // translation, translation_x, ...
	@:optional public var name: String;
	@:optional public var values: Float32Array; // translation
	@:optional public var value: Null<Float>; // translation_x
}

typedef TTrack = {
	public var target: String;
	public var frames: Uint32Array;
	public var values: Float32Array; // sampled - full matrix transforms, non-sampled - values
	@:optional public var ref_values: Array<Array<String>>; // ref values
}

typedef TProbeData = {
	public var name: String;
	public var type: String; // grid, planar
	public var strength: Float;
	@:optional public var irradiance: String; // Reference to TIrradiance blob
	@:optional public var radiance: String;
	@:optional public var radiance_mipmaps: Null<Int>;
}
