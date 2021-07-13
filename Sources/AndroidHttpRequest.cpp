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
    jclass activityClass = kinc_android_find_class(env, "arm.AndroidHttpRequest");

    jstring jstr = env->NewStringUTF(url);
    jbyteArray bytes_array = static_cast<jbyteArray>(env->CallStaticObjectMethod(activityClass, env->GetStaticMethodID(activityClass, "androidHttpRequest", "(Ljava/lang/String;)[B"), jstr));
    jsize num_bytes = env->GetArrayLength(bytes_array);
    jbyte* elements = env->GetByteArrayElements(bytes_array, NULL);
    if (elements != NULL) {
        callback(0, 200, (char *)elements, callbackdata);
    }
    env->ReleaseByteArrayElements(bytes_array, elements, JNI_ABORT);

    activity->vm->DetachCurrentThread();
}
