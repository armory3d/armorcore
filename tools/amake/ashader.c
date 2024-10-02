
// Simple text processing tool for converting shaders at runtime.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "iron_string.h"
#ifdef __linux__
int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version);
#endif

#ifdef _WIN32
#include <d3d11.h>
#include <D3Dcompiler.h>
#include <iron_array.h>
#include <iron_map.h>

char *hlsl_to_bin(char *source, char *shader_type, char *to) {

	char *type;
	if (strcmp(shader_type, "vert") == 0) {
		type = "vs_5_0";
	}
	else if (strcmp(shader_type, "frag") == 0) {
		type = "ps_5_0";
	}
	else {
		type = "gs_5_0";
	}

	ID3DBlob *error_message;
	ID3DBlob *shader_buffer;
	// UINT flags = D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_SKIP_VALIDATION;
	UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
	HRESULT hr = D3DCompile(source, strlen(source) + 1, NULL, NULL, NULL, "main", type, flags, 0, &shader_buffer, &error_message);
	if (hr != S_OK) {
		printf("%s\n", (char *)error_message->lpVtbl->GetBufferPointer(error_message));
		return JS_UNDEFINED;
	}

	ID3D11ShaderReflection *reflector = NULL;
	D3DReflect(shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer), &IID_ID3D11ShaderReflection, (void **)&reflector);

	int len = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	char *file = malloc(len * 2);
	int output_len = 0;

	bool has_bone = strstr(source, " bone :") != NULL;
	bool has_col = strstr(source, " col :") != NULL;
	bool has_nor = strstr(source, " nor :") != NULL;
	bool has_pos = strstr(source, " pos :") != NULL;
	bool has_tex = strstr(source, " tex :") != NULL;

	i32_map_t *attributes = i32_map_create();
	int index = 0;
	if (has_bone) i32_map_set(attributes, "bone", index++);
	if (has_col) i32_map_set(attributes, "col", index++);
	if (has_nor) i32_map_set(attributes, "nor", index++);
	if (has_pos) i32_map_set(attributes, "pos", index++);
	if (has_tex) i32_map_set(attributes, "tex", index++);
	if (has_bone) i32_map_set(attributes, "weight", index++);

	file[output_len] = (char)index;
	output_len += 1;

	any_array_t *keys = map_keys(attributes);
	for (int i = 0; i < keys->length; ++i) {
		strcpy(file + output_len, keys->buffer[i]);
		output_len += strlen(keys->buffer[i]);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = i32_map_get(attributes, keys->buffer[i]);
		output_len += 1;
	}

	D3D11_SHADER_DESC desc;
	reflector->lpVtbl->GetDesc(reflector, &desc);

	file[output_len] = desc.BoundResources;
	output_len += 1;
	for (int i = 0; i < desc.BoundResources; ++i) {
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		reflector->lpVtbl->GetResourceBindingDesc(reflector, i, &bindDesc);
		strcpy(file + output_len, bindDesc.Name);
		output_len += strlen(bindDesc.Name);
		file[output_len] = 0;
		output_len += 1;
		file[output_len] = bindDesc.BindPoint;
		output_len += 1;
	}

	ID3D11ShaderReflectionConstantBuffer *constants = reflector->lpVtbl->GetConstantBufferByName(reflector, "$Globals");
	D3D11_SHADER_BUFFER_DESC buffer_desc;
	hr = constants->lpVtbl->GetDesc(constants, &buffer_desc);
	if (hr == S_OK) {
		file[output_len] = buffer_desc.Variables;
		output_len += 1;
		for (int i = 0; i < buffer_desc.Variables; ++i) {
			ID3D11ShaderReflectionVariable *variable = constants->lpVtbl->GetVariableByIndex(constants, i);
			D3D11_SHADER_VARIABLE_DESC variable_desc;
			hr = variable->lpVtbl->GetDesc(variable, &variable_desc);
			if (hr == S_OK) {
				strcpy(file + output_len, variable_desc.Name);
				output_len += strlen(variable_desc.Name);
				file[output_len] = 0;
				output_len += 1;

				*(uint32_t *)(file + output_len) = variable_desc.StartOffset;
				output_len += 4;

				*(uint32_t *)(file + output_len) = variable_desc.Size;
				output_len += 4;

				D3D11_SHADER_TYPE_DESC type_desc;
				ID3D11ShaderReflectionType *type = variable->lpVtbl->GetType(variable);
				hr = type->lpVtbl->GetDesc(type, &type_desc);
				if (hr == S_OK) {
					file[output_len] = type_desc.Columns;
					output_len += 1;
					file[output_len] = type_desc.Rows;
					output_len += 1;
				}
				else {
					file[output_len] = 0;
					output_len += 1;
					file[output_len] = 0;
					output_len += 1;
				}
			}
		}
	}
	else {
		file[output_len] = 0;
		output_len += 1;
	}

	memcpy(file + output_len, (char *)shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer));
	output_len += shader_buffer->lpVtbl->GetBufferSize(shader_buffer);

	shader_buffer->lpVtbl->Release(shader_buffer);
	reflector->lpVtbl->Release(reflector);

	FILE *fp = fopen(to, "wb");
	fwrite(file, 1, output_len, fp);
	fclose(fp);
	free(file);
}

void hlslbin(const char *from, const char *to) {
	FILE *fp = fopen(from, "rb");
	fseek(fp , 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	char *source = malloc(size + 1);
	source[size] = 0;
	fread(source, size, 1, fp);
	fclose(fp);

	char *type;
	if (strstr(from, ".vert.")) {
		type = "vert";
	}
	else if (strstr(from, ".frag.")) {
		type = "frag";
	}
	else {
		type = "geom";
	}

	hlsl_to_bin(source, type, to);
}
#endif

static char out[128 * 1024];
static char line[1024];
static char tmp[1024];

static char *buffer;
static int buffer_size;
static int pos = 0;

typedef struct var {
	char name[64];
	char type[16];
} var_t;

char *numbers[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

var_t inputs[8];
int inputs_count = 0;

var_t outputs[8];
int outputs_count;

const char *version_glsl = "#version 450\n";
const char *version_essl = "#version 300 es\n";
const char *precision_essl = "precision highp float;\nprecision mediump int;\n";

const char *header_glsl = "#define GLSL\n\
#define textureArg(tex) sampler2D tex \n\
#define texturePass(tex) tex \n\
#define mul(a, b) b * a \n\
#define textureShared texture \n\
#define textureLodShared textureLod \n\
#define atan2(x, y) atan(y, x) \n\
";

// shared_sampler
const char *header_hlsl = "#define HLSL\n\
#define textureArg(tex) Texture2D tex,SamplerState tex ## _sampler\n\
#define texturePass(tex) tex,tex ## _sampler\n\
#define sampler2D Texture2D\n\
#define sampler3D Texture3D\n\
#define texture(tex, coord) tex.Sample(tex ## _sampler, coord)\n\
#define textureShared(tex, coord) tex.Sample(\" + shared_sampler + \", coord)\n\
#define textureLod(tex, coord, lod) tex.SampleLevel(tex ## _sampler, coord, lod)\n\
#define textureLodShared(tex, coord, lod) tex.SampleLevel(\" + shared_sampler + \", coord, lod)\n\
#define texelFetch(tex, coord, lod) tex.Load(float3(coord.xy, lod))\n\
uint2 _GetDimensions(Texture2D tex, uint lod) { uint x, y; tex.GetDimensions(x, y); return uint2(x, y); }\n\
#define textureSize _GetDimensions\n\
#define mod(a, b) (a % b)\n\
#define vec2 float2\n\
#define vec3 float3\n\
#define vec4 float4\n\
#define ivec2 int2\n\
#define ivec3 int3\n\
#define ivec4 int4\n\
#define mat2 float2x2\n\
#define mat3 float3x3\n\
#define mat4 float4x4\n\
#define dFdx ddx\n\
#define dFdy ddy\n\
#define inversesqrt rsqrt\n\
#define fract frac\n\
#define mix lerp\n\
";

// shared_sampler
const char *header_msl = "#define METAL\n\
#include <metal_stdlib>\n\
#include <simd/simd.h>\n\
using namespace metal;\n\
#define textureArg(tex) texture2d<float> tex,sampler tex ## _sampler\n\
#define texturePass(tex) tex,tex ## _sampler\n\
#define sampler2D texture2d<float>\n\
#define sampler3D texture3d<float>\n\
#define texture(tex, coord) tex.sample(tex ## _sampler, coord)\n\
#define textureShared(tex, coord) tex.sample(\" + shared_sampler + \", coord)\n\
#define textureLod(tex, coord, lod) tex.sample(tex ## _sampler, coord, level(lod))\n\
#define textureLodShared(tex, coord, lod) tex.sample(\" + shared_sampler + \", coord, level(lod))\n\
#define texelFetch(tex, coord, lod) tex.read(uint2(coord), uint(lod))\n\
float2 _getDimensions(texture2d<float> tex, uint lod) { return float2(tex.get_width(lod), tex.get_height(lod)); }\n\
#define textureSize _getDimensions\n\
#define mod(a, b) fmod(a, b)\n\
#define vec2 float2\n\
#define vec3 float3\n\
#define vec4 float4\n\
#define ivec2 int2\n\
#define ivec3 int3\n\
#define ivec4 int4\n\
#define mat2 float2x2\n\
#define mat3 float3x3\n\
#define mat4 float4x4\n\
#define dFdx dfdx\n\
#define dFdy dfdy\n\
#define inversesqrt rsqrt\n\
#define mul(a, b) b * a\n\
#define discard discard_fragment()\n\
";

static char *read_line() {
	int i = 0;
	while (true) {
		if (buffer[pos] == '\0') {
			return NULL;
		}

		line[i] = buffer[pos];
		i++;
		pos++;

		if (buffer[pos - 1] == '\n') {
			break;
		}
	}
	line[i] = '\0';
	return &line[0];
}

static void add_includes(char *file, int off) {
	char *_buffer = buffer;
	int _pos = pos;
	pos = off;

	FILE *fp = fopen(file, "rb");
	fseek(fp , 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	buffer = malloc(size + 1);
	buffer[size] = '\0';
	fread(buffer, size, 1, fp);
	fclose(fp);

	while (true) {
		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "#include ")) {
			char *rel = line + 10; // #include "
			rel[strlen(rel) - 1 - 1] = '\0'; // trailing "

			int last = string_last_index_of(file, "/") + 1;
			strncpy(tmp, file, last);
			tmp[last] = '\0';
			strcat(tmp, rel);

			add_includes(tmp, 0);
			line[0] = '\0';
		}

		strcat(out, line);
	}

	free(buffer);
	buffer = _buffer;
	pos = _pos;
}

static void add_header(char *shader_lang) {
	if (strcmp(shader_lang, "glsl") == 0 || strcmp(shader_lang, "essl") == 0 || strcmp(shader_lang, "spirv") == 0) {
		if (strcmp(shader_lang, "essl") == 0) {
			strcpy(out, version_essl);
			strcat(out, precision_essl);
		}
		else {
			strcpy(out, version_glsl);
		}
		strcat(out, header_glsl);
	}
	else if (strcmp(shader_lang, "hlsl") == 0) {
		strcpy(out, header_hlsl);
	}
	else if (strcmp(shader_lang, "msl") == 0) {
		strcpy(out, header_msl);
	}
}

static int sort_vars(var_t *a, var_t *b) {
	return strcmp(a->name, b->name);
}

static char *add_inputs(char *line) {
	memset(&inputs, 0, sizeof(inputs));
	inputs_count = 0;

	while (starts_with(line, "in ")) {
		line = line + 3; // "in "

		int pos = string_index_of(line, " ");
		strncat(inputs[inputs_count].type, line, pos);
		line += pos + 1;

		pos = string_index_of(line, ";");
		strncat(inputs[inputs_count].name, line, pos);

		inputs_count++;

		line = read_line();
	}

	qsort(inputs, inputs_count, sizeof(inputs[0]), sort_vars);

	strcat(out, "struct shader_input {\n");
	for (int i = 0; i < inputs_count; ++i) {
		strcat(out, "\t");
		strcat(out, inputs[i].type);
		strcat(out, " ");
		strcat(out, inputs[i].name);
		strcat(out, ": TEXCOORD");
		strcat(out, numbers[i]);
		strcat(out, ";\n");
	}

	if (string_index_of(buffer, "gl_VertexID") > -1) {
		strcat(out, "uint gl_VertexID: SV_VertexID\n");
	}

	if (string_index_of(buffer, "gl_InstanceID") > -1) {
		strcat(out, "uint gl_InstanceID: SV_InstanceID\n");
	}

	strcat(out, "};\n");

	return line;
}

static char *add_outputs(char *line, char *shader_type) {
	memset(&outputs, 0, sizeof(outputs));
	outputs_count = 0;

	while (starts_with(line, "out ")) {
		line = line + 4; // "out "

		int pos = string_index_of(line, " ");
		strncat(outputs[outputs_count].type, line, pos);
		line += pos + 1;

		pos = string_index_of(line, ";");
		strncat(outputs[outputs_count].name, line, pos);

		outputs_count++;

		line = read_line();
	}

	qsort(outputs, outputs_count, sizeof(outputs[0]), sort_vars);

	strcat(out, "struct shader_output {\n");
	for (int i = 0; i < outputs_count; ++i) {
		strcat(out, "\t");
		strcat(out, outputs[i].type);
		strcat(out, " ");
		strcat(out, outputs[i].name);

		if (strcmp(shader_type, "vert") == 0) {
			strcat(out, ": TEXCOORD");
		}
		else {
			strcat(out, ": SV_TARGET");
		}

		strcat(out, numbers[i]);
		strcat(out, ";\n");
	}

	if (strcmp(shader_type, "vert") == 0) {
		strcat(out, "\tvec4 gl_Position: SV_POSITION;\n");
	}

	strcat(out, "};\n");

	return line;
}

static void to_hlsl(const char *shader_type) {
	buffer = malloc(128 * 1024);
	strcpy(buffer, out);
	buffer_size = strlen(buffer);
	out[0] = '\0';
	pos = 0;

	while (true) {
		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "in ")) {
			line = add_inputs(line);
		}

		if (starts_with(line, "out ")) {
			line = add_outputs(line, shader_type);
		}

		if (starts_with(line, "uniform sampler")) {
			int len = strlen(line);
			int last = string_last_index_of(line, " ");
			strcpy(tmp, "SamplerState ");
			strncat(tmp, line + last + 1, len - last - 3); // - ";\n"
			strcat(tmp, "_sampler;\n");
			strcat(out, tmp);
		}

		if (starts_with(line, "void main")) {
			for (int i = 0; i < inputs_count; ++i) {
				strcat(out, inputs[i].type);
				strcat(out, " ");
				strcat(out, inputs[i].name);
				strcat(out, ";\n");
			}

			if (strcmp(shader_type, "vert") == 0) {
				strcat(out, "vec4 gl_Position;\n");
			}

			for (int i = 0; i < outputs_count; ++i) {
				strcat(out, outputs[i].type);
				strcat(out, " ");
				strcat(out, outputs[i].name);
				strcat(out, ";\n");
			}

			strcat(out, "shader_output main(shader_input stage_input) {\n");
			line = "";

			for (int i = 0; i < inputs_count; ++i) {
				strcat(out, "\t");
				strcat(out, inputs[i].name);
				strcat(out, " = stage_input.");
				strcat(out, inputs[i].name);
				strcat(out, ";\n");
			}
		}

		if (starts_with(line, "}") && pos == buffer_size) {
			strcat(out, "\tshader_output stage_output;\n");
			if (strcmp(shader_type, "vert") == 0) {
				strcat(out, "\tgl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n");
				strcat(out, "\ttstage_output.gl_Position = gl_Position;\n");
			}
			for (int i = 0; i < outputs_count; ++i) {
				strcat(out, "\tstage_output.");
				strcat(out, outputs[i].name);
				strcat(out, " = ");
				strcat(out, outputs[i].name);
				strcat(out, ";\n");
			}
			strcat(out, "\treturn stage_output;\n");
		}

		strcat(out, line);
	}

	free(buffer);
}

int ashader(char *shader_lang, char *from, char *to) {
	// shader_lang == glsl || essl || hlsl || msl || spirv
	const char *shader_type = string_index_of(from, ".vert") != -1 ? "vert" : "frag";

	add_header(shader_lang);
	add_includes(from, 13); // Skip #version 450\n

	FILE *fp = fopen(to, "wb");

	if (strcmp(shader_lang, "glsl") == 0 || strcmp(shader_lang, "essl") == 0) {
		fwrite(out, 1, strlen(out), fp);
	}

	#ifdef __linux__
	else if (strcmp(shader_lang, "spirv") == 0) {
		char *buf = malloc(1024 * 1024);
		int buf_len;
		krafix_compile(out, buf, &buf_len, "spirv", "linux", shader_type, -1);
		fwrite(buf, 1, buf_len, fp);
	}
	#endif

	#ifdef _WIN32
	else if (strcmp(shader_lang, "hlsl") == 0) {
		to_hlsl(shader_type);
		hlsl_to_bin(out, shader_type, to);
	}
	#endif

	#ifdef __APPLE__
	else if (strcmp(shader_lang, "msl") == 0) {
		// to_msl();
	}
	#endif

	return 0;
}
