
// Simple text processing tool for converting shaders at runtime.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "iron_string.h"

char out[128 * 1024];
char line[1024];

char *buffer;
int pos = 0;

const char *header_glsl = "#version 450\n\
#define GLSL\n\
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

char *read_line() {
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

void process_file(char *file, int off) {
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

            char path[512];
            int last = string_last_index_of(file, "/") + 1;
            strncpy(path, file, last);
            path[last] = '\0';
            strcat(path, rel);

            process_file(path, 0);
            line[0] = '\0';
        }

        strcat(out, line);
    }

    free(buffer);
    buffer = _buffer;
    pos = _pos;
}

// glsl from to
int ashader_compile(int argc, char **argv) {
    strcpy(out, header_glsl);

    process_file(argv[2], 13); // Skip #version 450\n

    FILE *fp = fopen(argv[3], "wb");
    fwrite(out, 1, strlen(out), fp);

    return 0;
}
