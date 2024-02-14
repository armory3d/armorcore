// #include <quickjs.h>

// int main(const int argc, const char **argv) {

//     JSRuntime *runtime = JS_NewRuntime();
//     JSContext *ctx = JS_NewContext(runtime);

//     JSValue result = JS_Eval(ctx, "5+2", 3, "mini.js", JS_EVAL_TYPE_GLOBAL);
//     printf("%d\n", JS_VALUE_GET_INT(result));

//     JS_FreeValue(ctx, result);
//     JS_RunGC(runtime);
//     return 0;
// }
