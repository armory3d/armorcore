#include "AndroidHttpRequest.h"
#include <kinc/system.h>
#include <jni.h>
#include <string.h>
#include <cstdlib>
#include <android_native_app_glue.h>

extern "C" ANativeActivity *kinc_android_get_activity(void);
extern "C" jclass kinc_android_find_class(JNIEnv *env, const char *name);

void android_http_request(const char *url, const char *path, const char *data, int port, bool secure, int method, const char *header,
                          kinc_http_callback_t callback, void *callbackdata) {
    ANativeActivity *activity = kinc_android_get_activity();
    JNIEnv *env;
    activity->vm->AttachCurrentThread(&env, nullptr);
    jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");

    jbyteArray bytes_array = env->CallStaticObjectMethod(koreActivityClass, env->GetStaticMethodID(koreActivityClass, "androidHttpRequest", "()[B"));
    jsize num_bytes = (*env)->GetArrayLength(env, bytes_array);
    jbyte* elements = (*env)->GetByteArrayElements(env, bytes_array, NULL);
    if (elements != NULL) {
        callback(0, 200, elements, callbackdata);
    }
    (*env)->ReleaseByteArrayElements(env, bytes_array, elements, JNI_ABORT);

    activity->vm->DetachCurrentThread();
}
