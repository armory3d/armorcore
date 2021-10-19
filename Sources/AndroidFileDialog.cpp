#include "AndroidFileDialog.h"
#include <kinc/pch.h>
#include <kinc/system.h>
#include <jni.h>
#include <string.h>
#include <cstdlib>
#include <android_native_app_glue.h>

extern "C" ANativeActivity *kinc_android_get_activity(void);
extern "C" jclass kinc_android_find_class(JNIEnv *env, const char *name);

extern "C" JNIEXPORT void JNICALL Java_tech_kode_kore_KoreActivity_onAndroidFilePicked(JNIEnv *env, jobject jobj, jstring jstr) {
	if (jstr == NULL) return;
	const char *str = env->GetStringUTFChars(jstr, 0);
	size_t len = strlen(str);
	wchar_t filePath[len + 1];
	mbstowcs(filePath, (char *)str, len);
	filePath[len] = 0;

	kinc_internal_drop_files_callback(filePath);
	env->ReleaseStringUTFChars(jstr, str);
}

void AndroidFileDialogOpen() {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	env->CallStaticVoidMethod(koreActivityClass, env->GetStaticMethodID(koreActivityClass, "pickFile", "()V"));
	activity->vm->DetachCurrentThread();
}

wchar_t *AndroidFileDialogSave() {
	return 0;
}

jstring android_permission_name(JNIEnv *env, const char *perm_name) {
	jclass ClassManifestpermission = env->FindClass("android/Manifest$permission");
	jfieldID lid_PERM = env->GetStaticFieldID(ClassManifestpermission, perm_name, "Ljava/lang/String;");
	jstring ls_PERM = (jstring)(env->GetStaticObjectField(ClassManifestpermission, lid_PERM));
	return ls_PERM;
}

bool android_has_permission(struct android_app *app, const char *perm_name) {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	bool result = false;
	jstring ls_PERM = android_permission_name(env, perm_name);
	jint PERMISSION_GRANTED = jint(-1);
	{
		jclass ClassPackageManager = env->FindClass("android/content/pm/PackageManager");
		jfieldID lid_PERMISSION_GRANTED = env->GetStaticFieldID(ClassPackageManager, "PERMISSION_GRANTED", "I");
		PERMISSION_GRANTED = env->GetStaticIntField(ClassPackageManager, lid_PERMISSION_GRANTED);
	}
	{
		jobject activity = app->activity->clazz;
		jclass ClassContext = env->FindClass("android/content/Context");
		jmethodID MethodcheckSelfPermission = env->GetMethodID(ClassContext, "checkSelfPermission", "(Ljava/lang/String;)I");
		jint int_result = env->CallIntMethod(activity, MethodcheckSelfPermission, ls_PERM);
		result = (int_result == PERMISSION_GRANTED);
	}
	activity->vm->DetachCurrentThread();
	return result;
}

void android_request_file_permissions(struct android_app *app) {
	ANativeActivity *activity = kinc_android_get_activity();
	JNIEnv *env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jobjectArray perm_array = env->NewObjectArray(2, env->FindClass("java/lang/String"), env->NewStringUTF(""));
	env->SetObjectArrayElement(perm_array, 0, android_permission_name(env, "READ_EXTERNAL_STORAGE"));
	env->SetObjectArrayElement(perm_array, 1, android_permission_name(env, "WRITE_EXTERNAL_STORAGE"));
	jobject jactivity = app->activity->clazz;
	jclass ClassActivity = env->FindClass("android/app/Activity");
	jmethodID MethodrequestPermissions = env->GetMethodID(ClassActivity, "requestPermissions", "([Ljava/lang/String;I)V");
	env->CallVoidMethod(jactivity, MethodrequestPermissions, perm_array, 0);
	activity->vm->DetachCurrentThread();
}

void android_check_permissions() {
	ANativeActivity *activity = kinc_android_get_activity();
	struct android_app *app = (struct android_app *)activity->instance;
	bool hasPermissions = android_has_permission(app, "READ_EXTERNAL_STORAGE") && android_has_permission(app, "WRITE_EXTERNAL_STORAGE");
	if (!hasPermissions) android_request_file_permissions(app);

	JNIEnv *env;
	activity->vm->AttachCurrentThread(&env, nullptr);
	jclass koreActivityClass = kinc_android_find_class(env, "tech.kinc.KincActivity");
	JNINativeMethod methodTable[] = {{"onAndroidFilePicked", "(Ljava/lang/String;)V", (void*)Java_tech_kode_kore_KoreActivity_onAndroidFilePicked}};
	int methodTableSize = sizeof(methodTable) / sizeof(methodTable[0]);
	env->RegisterNatives(koreActivityClass, methodTable, methodTableSize);
	activity->vm->DetachCurrentThread();
}
