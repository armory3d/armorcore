
// Mini-kmake based on https://github.com/Kode/kmake by RobDangerous
// gcc has lto enabled:
// ../../Kinc/make --compiler gcc --compile

#include <stdio.h>
#include <malloc.h>
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"

#ifdef _WIN32
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

    _execv(exec_argv[0], (char **)exec_argv);
    return JS_UNDEFINED;
}
#endif

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

    #ifdef _WIN32
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue kmake = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, kmake, "os_exec_win", JS_NewCFunction(ctx, js_os_exec_win, "os_exec_win", 1));
    JS_SetPropertyStr(ctx, global_obj, "kmake", kmake);
    JS_FreeValue(ctx, global_obj);
    #endif

    JSValue ret = JS_Eval(ctx, buffer, size, "make.js", JS_EVAL_TYPE_MODULE);

    if (JS_IsException(ret)) {
        js_std_dump_error(ctx);
        JS_ResetUncatchableError(ctx);
    }

    JS_RunGC(runtime);
    free(buffer);
    return 0;
}
