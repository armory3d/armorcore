
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

static void write_attrib(char *file, int *output_len, const char *attrib, int index) {
	if (index > -1) {
		strcpy(file + (*output_len), attrib);
		(*output_len) += strlen(attrib);
		file[(*output_len)] = 0;
		(*output_len) += 1;
		file[(*output_len)] = index;
		(*output_len) += 1;
	}
}

char *hlsl_to_bin(char *source, char *shader_type, char *to) {
	char *type;
	if (string_equals(shader_type, "vert")) {
		type = "vs_5_0";
	}
	else if (string_equals(shader_type, "frag")) {
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
		return NULL;
	}

	ID3D11ShaderReflection *reflector = NULL;
	D3DReflect(shader_buffer->lpVtbl->GetBufferPointer(shader_buffer), shader_buffer->lpVtbl->GetBufferSize(shader_buffer), &IID_ID3D11ShaderReflection, (void **)&reflector);

	int len = shader_buffer->lpVtbl->GetBufferSize(shader_buffer);
	char *file = malloc(len * 2);
	int output_len = 0;

	bool has_bone = strstr(source, " bone:") != NULL;
	bool has_col = strstr(source, " col:") != NULL;
	bool has_nor = strstr(source, " nor:") != NULL;
	bool has_pos = strstr(source, " pos:") != NULL;
	bool has_tex = strstr(source, " tex:") != NULL;

	int ibone = -1;
	int icol = -1;
	int inor = -1;
	int ipos = -1;
	int itex = -1;
	int iweight = -1;

	int index = 0;
	if (has_bone) ibone = index++;
	if (has_col) icol = index++;
	if (has_nor) inor = index++;
	if (has_pos) ipos = index++;
	if (has_tex) itex = index++;
	if (has_bone) iweight = index++;

	file[output_len] = (char)index;
	output_len += 1;

	write_attrib(file, &output_len, "bone", ibone);
	write_attrib(file, &output_len, "col", icol);
	write_attrib(file, &output_len, "nor", inor);
	write_attrib(file, &output_len, "pos", ipos);
	write_attrib(file, &output_len, "tex", itex);
	write_attrib(file, &output_len, "weight", iweight);

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
static int out_pos = 0;
static char line[1024];
static char tmp[1024];

static char *buffer;
static int buffer_size;
static int buffer_pos = 0;

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
#define mul(a, b) b * a \n\
#define atan2(x, y) atan(y, x) \n\
";

const char *header_hlsl = "#define HLSL\n\
#define sampler2D Texture2D\n\
#define sampler3D Texture3D\n\
#define texture(tex, coord) tex.Sample(tex ## _sampler, coord)\n\
#define textureLod(tex, coord, lod) tex.SampleLevel(tex ## _sampler, coord, lod)\n\
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

const char *header_msl = "#define METAL\n\
#include <metal_stdlib>\n\
#include <simd/simd.h>\n\
using namespace metal;\n\
#define sampler2D texture2d<float>\n\
#define sampler3D texture3d<float>\n\
#define texture(tex, coord) tex.sample(tex ## _sampler, coord)\n\
#define textureLod(tex, coord, lod) tex.sample(tex ## _sampler, coord, level(lod))\n\
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
		if (buffer[buffer_pos] == '\0') {
			return NULL;
		}

		line[i] = buffer[buffer_pos];
		i++;
		buffer_pos++;

		#ifdef _WIN32
		if (line[i - 1] == '\r') {
			line[i - 1] = '\n';
			buffer_pos++; // Skip \n
			break;
		}
		#endif

		if (line[i - 1] == '\n') {
			break;
		}
	}

	line[i] = '\0';
	return &line[0];
}

static void w(char *str) {
	strcpy(out + out_pos, str);
	out_pos += strlen(str);
}

static void write_includes(char *file, int off) {
	char *_buffer = buffer;
	int _pos = buffer_pos;
	buffer_pos = off;

	FILE *fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
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

			write_includes(tmp, 0);
			line[0] = '\0';
		}

		w(line);
	}

	free(buffer);
	buffer = _buffer;
	buffer_pos = _pos;
}

static void write_header(char *shader_lang) {
	if (string_equals(shader_lang, "glsl") ||
		string_equals(shader_lang, "essl") ||
		string_equals(shader_lang, "spirv")
	) {
		if (string_equals(shader_lang, "essl")) {
			w(version_essl);
			w(precision_essl);
		}
		else {
			w(version_glsl);
		}
		w(header_glsl);
	}
	else if (string_equals(shader_lang, "hlsl")) {
		w(header_hlsl);
	}
	else if (string_equals(shader_lang, "msl")) {
		w(header_msl);
	}
}

static int sort_vars(const void *_a, const void *_b) {
	var_t *a = _a;
	var_t *b = _b;
	return strcmp(a->name, b->name);
}

static void read_vars(char *line, var_t *vars, int *vars_count, char *start) {
	*vars_count = 0;

	while (starts_with(line, start)) {
		memset(&vars[*vars_count], 0, sizeof(var_t));
		line = line + strlen(start);

		int pos = string_index_of(line, " ");
		strncat(vars[*vars_count].type, line, pos);
		line += pos + 1;

		pos = string_index_of(line, ";");
		strncat(vars[*vars_count].name, line, pos);

		(*vars_count)++;
		line = read_line();
	}

	qsort(vars, *vars_count, sizeof(var_t), sort_vars);
}

static char *write_inputs(char *line) {
	read_vars(line, inputs, &inputs_count, "in ");

	w("struct shader_input {\n");
	for (int i = 0; i < inputs_count; ++i) {
		w("\t");
		w(inputs[i].type);
		w(" ");
		w(inputs[i].name);
		w(": TEXCOORD");
		w(numbers[i]);
		w(";\n");
	}

	if (string_index_of(buffer, "gl_VertexID") > -1) {
		w("\tuint gl_VertexID: SV_VertexID;\n");
	}
	if (string_index_of(buffer, "gl_InstanceID") > -1) {
		w("\tuint gl_InstanceID: SV_InstanceID;\n");
	}

	w("};\n");

	return line;
}

static char *write_outputs(char *line, char *shader_type) {
	read_vars(line, outputs, &outputs_count, "out ");

	w("struct shader_output {\n");
	for (int i = 0; i < outputs_count; ++i) {
		w("\t");
		w(outputs[i].type);
		w(" ");
		w(outputs[i].name);

		if (string_equals(shader_type, "vert")) {
			w(": TEXCOORD");
		}
		else {
			w(": SV_TARGET");
		}

		w(numbers[i]);
		w(";\n");
	}

	if (string_equals(shader_type, "vert")) {
		w("\tvec4 gl_Position: SV_POSITION;\n");
	}

	w("};\n");

	return line;
}

static void write_return(const char *shader_type) {
	w("\tshader_output stage_output;\n");
	if (string_equals(shader_type, "vert")) {
		w("\tgl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n");
		w("\tstage_output.gl_Position = gl_Position;\n");
	}
	for (int i = 0; i < outputs_count; ++i) {
		w("\tstage_output.");
		w(outputs[i].name);
		if (out[out_pos - 1] == ']') {
			out_pos -= 3; // Erase [i]
		}
		w(" = ");
		w(outputs[i].name);
		if (out[out_pos - 1] == ']') {
			out_pos -= 3;
		}
		w(";\n");
	}
	w("\treturn stage_output;\n");
}

static void to_hlsl(const char *shader_type) {
	buffer = malloc(128 * 1024);
	strcpy(buffer, out);
	buffer_size = strlen(buffer);
	buffer_pos = 0;
	out[0] = '\0';
	out_pos = 0;

	while (true) {
		char *line = read_line();
		if (line == NULL) {
			break;
		}

		if (starts_with(line, "in ")) {
			line = write_inputs(line);

			for (int i = 0; i < inputs_count; ++i) {
				w("static ");
				w(inputs[i].type);
				w(" ");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("static uint gl_VertexID;\n");
			}
			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("static uint gl_InstanceID;\n");
			}
		}

		if (starts_with(line, "out ")) {
			line = write_outputs(line, shader_type);

			if (string_equals(shader_type, "vert")) {
				w("static vec4 gl_Position;\n");
			}

			for (int i = 0; i < outputs_count; ++i) {
				w("static ");
				w(outputs[i].type);
				w(" ");
				w(outputs[i].name);
				w(";\n");
			}
		}

		if (starts_with(line, "uniform sampler")) {
			int len = strlen(line);
			int last = string_last_index_of(line, " ");
			strcpy(tmp, "SamplerState ");
			strncat(tmp, line + last + 1, len - last - 3); // - ";\n"
			strcat(tmp, "_sampler;\n");
			w(tmp);
		}

		if (starts_with(line, "const ")) {
			w("static ");
		}

		if (starts_with(line, "vec") ||
			starts_with(line, "float ") ||
			starts_with(line, "int ")
		) {
			if (ends_with(line, ";\n")) {
				w("static ");
			}
		}

		if (ends_with(line, "return;\n")) {
			write_return(shader_type);
			line = "";
		}

		if (starts_with(line, "void main")) {
			w("shader_output main(shader_input stage_input) {\n");
			line = "";

			for (int i = 0; i < inputs_count; ++i) {
				w("\t");
				w(inputs[i].name);
				w(" = stage_input.");
				w(inputs[i].name);
				w(";\n");
			}

			if (string_index_of(buffer, "gl_VertexID") > -1) {
				w("\tgl_VertexID = stage_input.gl_VertexID;\n");
			}

			if (string_index_of(buffer, "gl_InstanceID") > -1) {
				w("\tgl_InstanceID = stage_input.gl_InstanceID;\n");
			}
		}

		if (starts_with(line, "}") && buffer_pos == buffer_size) {
			write_return(shader_type);
		}

		w(line);
	}

	free(buffer);
}

#ifdef __APPLE__
static void to_msl(const char *shader_type) {
}
#endif

int ashader(char *shader_lang, char *from, char *to) {
	// shader_lang == glsl || essl || hlsl || msl || spirv
	const char *shader_type = string_index_of(from, ".vert") != -1 ? "vert" : "frag";

	#ifdef _WIN32
	char from_[512];
	strcpy(from_, from);
	int len = strlen(from_);
	for (int i = 0; i < len; ++i) {
		if (from_[i] == '\\') {
			from_[i] = '/';
		}
	}
	from = from_;
	#endif

	out[0] = '\0';
	out_pos = 0;
	write_header(shader_lang);
	write_includes(from, 13); // Skip #version 450\n

	if (string_equals(shader_lang, "glsl") ||
		string_equals(shader_lang, "essl")
	) {
		FILE *fp = fopen(to, "wb");
		fwrite(out, 1, strlen(out), fp);
		fclose(fp);
	}

	#ifdef __linux__
	else if (string_equals(shader_lang, "spirv")) {
		char *buf = malloc(1024 * 1024);
		int buf_len;
		krafix_compile(out, buf, &buf_len, "spirv", "linux", shader_type, -1);

		FILE *fp = fopen(to, "wb");
		fwrite(buf, 1, buf_len, fp);
		fclose(fp);
	}
	#endif

	#ifdef _WIN32
	else if (string_equals(shader_lang, "hlsl")) {
		to_hlsl(shader_type);

		FILE *fp = fopen(to, "wb");
		fwrite(out, 1, strlen(out), fp); // Write .hlsl
		fclose(fp);

		char to_[512];
		strcpy(to_, to);
		to_[strlen(to_) - 4] = '\0';
		strcat(to_, "d3d11");
		hlsl_to_bin(out, shader_type, to_);
	}
	#endif

	#ifdef __APPLE__
	else if (string_equals(shader_lang, "msl")) {
		to_msl(shader_type);
	}
	#endif

	return 0;
}
