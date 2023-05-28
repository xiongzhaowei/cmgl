//
// Created by 熊朝伟 on 2020-03-20.
//

#include "defines.h"
#include <android/looper.h>

OMP_RENDER_ANDROID_USING_NAMESPACE

static JavaVM *g_javaVM;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
    JNIEnv *env;
    g_javaVM = jvm;

    if (jvm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_ERR;
    }

    if (!registerNatives(env, AndroidRenderContext::className, AndroidRenderContext::nativeMethod)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved) {
    JNIEnv *env = NULL;
    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) return;
    if (!env) return;

    unregisterNatives(env, AndroidRenderContext::className);
}

JavaVM *android::javaVM() {
    return g_javaVM;
}

JNIEnv *android::javaEnv() {
    JNIEnv *env;
    JavaVM *jvm = javaVM();
    if (!jvm) {
        LOGD("couldn't get java virtual machine");
        assert(false);
        return NULL;
    }

    int getEnvStat = jvm->GetEnv((void **) &env, JNI_VERSION_1_4);

    if (getEnvStat == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, NULL) != 0) {
            LOGD("couldn't get environment using GetEnv()");
            assert(false);
            return NULL;
        }
    } else if (getEnvStat != JNI_OK) {
        LOGD("couldn't get environment using GetEnv()");
        assert(false);
        return NULL;
    }

    return env;
}

jobject android::assetManager() {
    JNIEnv *env = javaEnv();
    if (!env) return nullptr;

    jclass clazz = env->FindClass("com/cmcm/cmgl/CMGLThreadContext");
    if (!clazz) return nullptr;

    jmethodID methodid = env->GetStaticMethodID(clazz, "getCurrentAssetManager", "()Landroid/content/res/AssetManager;");
    if (!methodid) return nullptr;

    return env->CallStaticObjectMethod(clazz, methodid);
}

bool android::registerNatives(JNIEnv *env, const char *className, const JNINativeMethod *methods, size_t count) {
    bool result = false;
    jclass cls = env->FindClass(className);
    if (cls) {
        if (0 == env->RegisterNatives(cls, methods, count)) {
            result = true;
        }
        env->DeleteLocalRef(cls);
    }
    return result;
}

bool android::unregisterNatives(JNIEnv *env, const char *className) {
    bool result = false;
    jclass cls = env->FindClass(className);
    if (cls) {
        if (0 == env->UnregisterNatives(cls)) {
            result = true;
        }
        env->DeleteLocalRef(cls);
    }
    return result;
}
