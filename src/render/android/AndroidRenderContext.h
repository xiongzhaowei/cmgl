//
// Created by 熊朝伟 on 2020-03-19.
//

#pragma once

OMP_RENDER_ANDROID_NAMESPACE_BEGIN

class AndroidFramebuffer;

class AndroidRenderContext : public egl::EGLRenderContext {
public:
    static AndroidRenderContext *fromObject(JNIEnv *env, jobject obj);
    static void setHandle(JNIEnv *env, jobject obj, AndroidRenderContext *context);

    static void JNICALL JNI_initialize(JNIEnv *env, jobject instance);
    static void JNICALL JNI_terminate(JNIEnv *env, jobject instance);
    static void JNICALL JNI_setTarget(JNIEnv *env, jobject instance, jobject surface);
    static void JNICALL JNI_render(JNIEnv *env, jobject instance);
    static void JNICALL JNI_pause(JNIEnv *env, jobject instance);
    static void JNICALL JNI_resume(JNIEnv *env, jobject instance);

    static const char className[];
    static const JNINativeMethod nativeMethod[6];
};

OMP_RENDER_ANDROID_NAMESPACE_END
