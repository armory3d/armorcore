
// ../../make --compiler gcc --compile (gcc has lto enabled)

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"
#include "../../sources/kinc/libs/lz4x.h"
#define STBI_KEEP_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "../../sources/kinc/libs/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../sources/libs/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

typedef struct image {
    void *data;
    int width;
    int height;
    bool is_hdr;
} image_t;

static bool ends_with(const char *s, const char *end) {
	size_t len_s = strlen(s);
	size_t len_end = strlen(end);
	return strncmp(s + len_s - len_end, end, len_end) == 0;
}

static image_t read_png_jpg(const char *filename) {
    int width, height, n;
    char *data = stbi_load(filename, &width, &height, &n, 4);
    image_t img = { .data = data, .width = width, .height = height, .is_hdr = false };
    return img;
}

static image_t read_hdr(const char *filename) {
	int width, height, n;
    float *data = stbi_loadf(filename, &width, &height, &n, 4);
    image_t img = { .data = data, .width = width, .height = height, .is_hdr = true };
    return img;
}

static void *scale_image(image_t img, int width, int height) {
    unsigned char *scaled = malloc(width * height * 4);
    stbir_resize_uint8_generic(img.data, img.width, img.height, img.width * 4, scaled, width, height, width * 4, 4, 3, 0,
                               STBIR_EDGE_CLAMP, STBIR_FILTER_BOX, STBIR_COLORSPACE_SRGB, 0);
    return scaled;
}

// Courtesy of https://github.com/Kode/kraffiti

static void write_ico_header(FILE *file) {
	fputc(0, file);
	fputc(0, file);
    short type_ico = 1;
    fwrite(&type_ico, 1, 2, file);
    short img_count = 5;
    fwrite(&img_count, 1, 2, file);
}

static void write_ico_entry(FILE *file, int width, int height, int size, int offset) {
	fputc(width == 256 ? 0 : width, file);
	fputc(height == 256 ? 0 : height, file);
	fputc(0, file);
	fputc(0, file);
    short color_planes = 1;
    fwrite(&color_planes, 1, 2, file);
    short bpp = 32;
    fwrite(&bpp, 1, 2, file);
    fwrite(&size, 1, 4, file);
    fwrite(&offset, 1, 4, file);
}

static JSValue js_export_ico(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    image_t img = read_png_jpg(from);
    FILE *file = fopen(to, "wb");

    unsigned char *data256 = scale_image(img, 256, 256);
    unsigned char *data48 = scale_image(img, 48, 48);
    unsigned char *data32 = scale_image(img, 32, 32);
    unsigned char *data24 = scale_image(img, 24, 24);
    unsigned char *data16 = scale_image(img, 16, 16);
    int comp = 4;
    int png256_len;
    int png48_len;
    int png32_len;
    int png24_len;
    int png16_len;
    unsigned char *png256 = stbi_write_png_to_mem(data256, 256 * comp, 256, 256, comp, &png256_len);
    unsigned char *png48 = stbi_write_png_to_mem(data48, 48 * comp, 48, 48, comp, &png48_len);
    unsigned char *png32 = stbi_write_png_to_mem(data32, 32 * comp, 32, 32, comp, &png32_len);
    unsigned char *png24 = stbi_write_png_to_mem(data24, 24 * comp, 24, 24, comp, &png24_len);
    unsigned char *png16 = stbi_write_png_to_mem(data16, 16 * comp, 16, 16, comp, &png16_len);

    write_ico_header(file);
    const int ico_header_size = 6;
    const int ico_entry_size = 16;
	int ico_offset = ico_header_size + ico_entry_size * 5;
	write_ico_entry(file, 16, 16, png16_len, ico_offset);
	ico_offset += png16_len;
	write_ico_entry(file, 24, 24, png24_len, ico_offset);
	ico_offset += png24_len;
	write_ico_entry(file, 32, 32, png32_len, ico_offset);
	ico_offset += png32_len;
	write_ico_entry(file, 48, 48, png48_len, ico_offset);
	ico_offset += png48_len;
	write_ico_entry(file, 256, 256, png256_len, ico_offset);

    fwrite(png16, 1, png16_len, file);
    fwrite(png24, 1, png24_len, file);
    fwrite(png32, 1, png32_len, file);
    fwrite(png48, 1, png48_len, file);
    fwrite(png256, 1, png256_len, file);
	fclose(file);
    return JS_UNDEFINED;
}

static JSValue js_export_png(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);
    int32_t width;
    JS_ToInt32(ctx, &width, argv[2]);
    int32_t height;
    JS_ToInt32(ctx, &height, argv[3]);

    image_t img = read_png_jpg(from);

    if (width > 0 && height > 0) {
        unsigned char *scaled = scale_image(img, width, height);
        free(img.data);
        img.data = scaled;
    }
    else {
        width = img.width;
        height = img.height;
    }

    int comp = 4;
    stbi_write_png(to, width, height, comp, (unsigned char *)img.data, width * comp);
    free(img.data);
    return JS_UNDEFINED;
}

static void write_k(int width, int height, const char *format, char *data, int size, const char *filename) {
	FILE *file = fopen(filename, "wb");
	fputc((unsigned char)width, file);
	fputc((unsigned char)(width >> 8), file);
	fputc((unsigned char)(width >> 16), file);
	fputc((unsigned char)(width >> 24), file);
	fputc((unsigned char)height, file);
	fputc((unsigned char)(height >> 8), file);
	fputc((unsigned char)(height >> 16), file);
	fputc((unsigned char)(height >> 24), file);
	fputc(format[0], file);
	fputc(format[1], file);
	fputc(format[2], file);
	fputc(format[3], file);
	fwrite(data, 1, size, file);
	fclose(file);
}

static JSValue js_export_k(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *from = JS_ToCString(ctx, argv[0]);
    const char *to = JS_ToCString(ctx, argv[1]);

    image_t img;
	if (ends_with(from, ".hdr")) {
		img = read_hdr(from);
    }
	else {
	    img = read_png_jpg(from);
    }

    int pixel_size = img.is_hdr ? 16 : 4;
    int max = LZ4_compress_bound(img.width * img.height * pixel_size);
    char *compressed = malloc(max);
    int compressed_size = LZ4_compress_default((char *)img.data, compressed, img.width * img.height * pixel_size, max);
    write_k(img.width, img.height, img.is_hdr ? "LZ4F" : "LZ4 ", compressed, compressed_size, to);
    free(compressed);
    free(img.data);
    return JS_UNDEFINED;
}

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
static JSValue js_os_exec_win(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    JSValue args = argv[0];
    JSValue val = JS_GetPropertyStr(ctx, args, "length");

    uint32_t exec_argc;
    JS_ToUint32(ctx, &exec_argc, val);

    char **exec_argv = js_mallocz(ctx, sizeof(exec_argv[0]) * (exec_argc + 1));
    for(int i = 0; i < exec_argc; i++) {
        val = JS_GetPropertyUint32(ctx, args, i);
        exec_argv[i] = JS_ToCString(ctx, val);
        JS_FreeValue(ctx, val);
    }
    exec_argv[exec_argc] = NULL;

    if (argc >= 2) {
        JSValue options = argv[1];
        val = JS_GetPropertyStr(ctx, options, "cwd");
        if (!JS_IsUndefined(val)) {
            char *cwd = JS_ToCString(ctx, val);
            JS_FreeValue(ctx, val);
            _chdir(cwd);
        }
    }

    char cmd[1024];
    cmd[0] = 0;
    for (int i = 0; i < exec_argc; ++i) {
        strcat(cmd, exec_argv[i]);
        strcat(cmd, " ");
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return JS_UNDEFINED;
}
#endif

void alang(char *source, char *output);
static JSValue js_alang(JSContext *ctx, JSValue this_val, int argc, JSValue *argv) {
    const char *source = JS_ToCString(ctx, argv[0]);
    const char *output = JS_ToCString(ctx, argv[1]);
    alang(source, output);
    return JS_UNDEFINED;
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    fseek(fp , 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    char *buffer = malloc(size + 1);
    buffer[size] = 0;
    fread(buffer, size, 1, fp);
    fclose(fp);

    JSRuntime *runtime = JS_NewRuntime();
    JSContext *ctx = JS_NewContext(runtime);

    js_std_add_helpers(ctx, argc, argv);
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue amake = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, amake, "export_k", JS_NewCFunction(ctx, js_export_k, "export_k", 2));
    JS_SetPropertyStr(ctx, amake, "export_ico", JS_NewCFunction(ctx, js_export_ico, "export_ico", 2));
    JS_SetPropertyStr(ctx, amake, "export_png", JS_NewCFunction(ctx, js_export_png, "export_png", 4));
    #ifdef _WIN32
    JS_SetPropertyStr(ctx, amake, "os_exec_win", JS_NewCFunction(ctx, js_os_exec_win, "os_exec_win", 1));
    #endif
    JS_SetPropertyStr(ctx, amake, "alang", JS_NewCFunction(ctx, js_alang, "alang", 2));

    JS_SetPropertyStr(ctx, global_obj, "amake", amake);
    JS_FreeValue(ctx, global_obj);

    JSValue ret = JS_Eval(ctx, buffer, size, "make.js", JS_EVAL_TYPE_MODULE);

    if (JS_IsException(ret)) {
        js_std_dump_error(ctx);
        JS_ResetUncatchableError(ctx);
    }

    JS_RunGC(runtime);
    free(buffer);
    return 0;
}
