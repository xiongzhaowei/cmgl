//
// Created by 熊朝伟 on 2020-03-19.
//

#include "../egl/defines.h"
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>

#define OMP_RENDER_ANDROID_NAMESPACE_BEGIN    OMP_RENDER_NAMESPACE_BEGIN namespace android {
#define OMP_RENDER_ANDROID_NAMESPACE_END      } OMP_RENDER_NAMESPACE_END
#define OMP_RENDER_ANDROID_USING_NAMESPACE    OMP_RENDER_USING_NAMESPACE using namespace OMP_RENDER_NAMESPACE_PREFIX android;


OMP_RENDER_ANDROID_NAMESPACE_BEGIN

JavaVM *javaVM();
JNIEnv *javaEnv();
jobject assetManager();

bool registerNatives(JNIEnv *env, const char *className, const JNINativeMethod *methods, size_t count);
bool unregisterNatives(JNIEnv *env, const char *className);

template <size_t count>
bool registerNatives(JNIEnv *env, const char *className, JNINativeMethod const (&methods)[count])
{ return registerNatives(env, className, methods, count); }

OMP_RENDER_ANDROID_NAMESPACE_END


#include "AndroidRenderContext.h"
#include "AndroidWindow.h"
