//
// Created by 熊朝伟 on 2020/3/25.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

VideoSource::VideoSource() : _isNeedsUpdate(false) {
}

vec2 VideoSource::size() const {
    return _size;
}

#ifdef __ANDROID__
VideoSource *VideoSource::fromObject(JNIEnv *env, jobject obj) {
    jclass objectClass = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(objectClass, "handle", "J");
    jlong handle = env->GetLongField(obj, field);
    return reinterpret_cast<VideoSource *>(handle);
}

void VideoSource::setHandle(JNIEnv *env, jobject obj, VideoSource *source) {
    jlong handle = reinterpret_cast<jlong>(source);
    jclass objectClass = env->GetObjectClass(obj);
    jfieldID field = env->GetFieldID(objectClass, "handle", "J");
    env->SetLongField(obj, field, handle);
}
#endif
