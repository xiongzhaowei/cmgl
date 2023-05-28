//
// Created by 熊朝伟 on 2020-03-19.
//

#include "defines.h"
#include <android/looper.h>

OMP_RENDER_ANDROID_USING_NAMESPACE

const char AndroidRenderContext::className[] = "com/omplayer/OMPRenderContext";

const JNINativeMethod AndroidRenderContext::nativeMethod[] = {
        { "initialize", "()V",                          (void*)&JNI_initialize },
        { "terminate",  "()V",                          (void*)&JNI_terminate },
        { "setTarget",  "(Landroid/view/Surface;)V",    (void*)&JNI_setTarget },
        { "render",     "()V",                          (void*)&JNI_render },
        { "pause",      "()V",                          (void*)&JNI_pause },
        { "resume",     "()V",                          (void*)&JNI_resume },
};

AndroidRenderContext *AndroidRenderContext::fromObject(JNIEnv *env, jobject obj) {
    jclass objectClass = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(objectClass, "handle", "J");
    jlong handle = env->GetLongField(obj, field);
    return reinterpret_cast<AndroidRenderContext *>(handle);
}

void AndroidRenderContext::setHandle(JNIEnv *env, jobject obj, AndroidRenderContext *context) {
    jlong handle = reinterpret_cast<jlong>(context);
    jclass objectClass = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(objectClass, "handle", "J");
    env->SetLongField(obj, field, handle);
}

void JNICALL AndroidRenderContext::JNI_initialize(JNIEnv *env, jobject instance) {
    assert(AndroidRenderContext::fromObject(env, instance) == nullptr);
    AndroidRenderContext *player = new AndroidRenderContext;
    player->retain();
    AndroidRenderContext::setHandle(env, instance, player);
    player->load();
}

void JNICALL AndroidRenderContext::JNI_terminate(JNIEnv *env, jobject instance) {
    RefPtr<AndroidRenderContext> player = AndroidRenderContext::fromObject(env, instance);
    if (!player) return;
    AndroidRenderContext::setHandle(env, instance, nullptr);
    player->unload();
    player->release();
}

void JNICALL AndroidRenderContext::JNI_setTarget(JNIEnv *env, jobject instance, jobject surface) {
    RefPtr<AndroidRenderContext> player = AndroidRenderContext::fromObject(env, instance);
    if (!player) return;
    player->makeCurrent();
    if (surface) {
        RefPtr<AndroidWindow> window = new AndroidWindow(ANativeWindow_fromSurface(env, surface));
        player->setTarget(window);
    } else {
        RefPtr<RenderTarget> target = player->target();
        player->setTarget(nullptr);
    }
}

void JNICALL AndroidRenderContext::JNI_render(JNIEnv *env, jobject instance) {
    RefPtr<AndroidRenderContext> player = AndroidRenderContext::fromObject(env, instance);
    if (!player) return;
    if (!player->target()) return;

    player->makeCurrent();
    player->render();
}

void JNICALL AndroidRenderContext::JNI_pause(JNIEnv *env, jobject instance) {
    RefPtr<AndroidRenderContext> player = AndroidRenderContext::fromObject(env, instance);
    if (!player) return;

    player->unload();
}

void JNICALL AndroidRenderContext::JNI_resume(JNIEnv *env, jobject instance) {
    RefPtr<AndroidRenderContext> player = AndroidRenderContext::fromObject(env, instance);
    if (!player) return;

    player->load();
}
