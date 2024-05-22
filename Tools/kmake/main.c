
// Mini-kmake based on https://github.com/Kode/kmake by RobDangerous
// gcc has lto enabled:
// ../../Kinc/make --compiler gcc --compile

#include <stdio.h>
#include <malloc.h>
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"

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

    // JSValue global_obj = JS_GetGlobalObject(ctx);
    // JSValue kmake = JS_NewObject(ctx);
    // JS_SetPropertyStr(ctx, kmake, "log", JS_NewCFunction(ctx, js_print, "log", 1));
    // JS_SetPropertyStr(ctx, global_obj, "kmake", kmake);
    // JS_FreeValue(ctx, global_obj);

    JSValue ret = JS_Eval(ctx, buffer, size, "make.js", JS_EVAL_TYPE_MODULE);

    if (JS_IsException(ret)) {
        js_std_dump_error(ctx);
        JS_ResetUncatchableError(ctx);
    }

    JS_RunGC(runtime);
    free(buffer);
    return 0;
}
