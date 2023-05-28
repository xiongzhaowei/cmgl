//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

#include "../defines.h"

#ifndef DEFINE_OMP_RENDER_NAMESPACE
    #define DEFINE_OMP_RENDER_NAMESPACE     1
#endif

#if DEFINE_OMP_RENDER_NAMESPACE && !defined(OMP_RENDER_NAMESPACE_NAME)
    #define OMP_RENDER_NAMESPACE_NAME       render
#endif

#ifdef OMP_RENDER_NAMESPACE_NAME
    #define OMP_RENDER_NAMESPACE_BEGIN      OMP_NAMESPACE_BEGIN namespace OMP_RENDER_NAMESPACE_NAME {
    #define OMP_RENDER_NAMESPACE_END        } OMP_NAMESPACE_END
    #define OMP_RENDER_NAMESPACE_PREFIX     OMP_NAMESPACE_PREFIX OMP_RENDER_NAMESPACE_NAME ::
    #define OMP_RENDER_USING_NAMESPACE      OMP_USING_NAMESPACE using namespace OMP_NAMESPACE_PREFIX OMP_RENDER_NAMESPACE_NAME;
#else
    #define OMP_RENDER_NAMESPACE_BEGIN
    #define OMP_RENDER_NAMESPACE_END
    #define OMP_RENDER_NAMESPACE_PREFIX
    #define OMP_RENDER_USING_NAMESPACE
#endif

#ifdef __cplusplus
OMP_RENDER_NAMESPACE_BEGIN
#ifndef _MSC_VER
using namespace glm;
#else
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::mat3;
using glm::mat4;
using glm::identity;
using glm::quat;
using glm::mat4_cast;
#endif
OMP_RENDER_NAMESPACE_END
#endif

#ifdef __ANDROID__

#include <android/log.h>

#define TAG "OMPlayer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG ,__VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG ,__VA_ARGS__)

#elif defined(__APPLE__) ||  defined(_WIN32)

#define TAG "OMPlayer"
#if defined(DEBUG) || !defined(NDEBUG)
#define __apple_log_print(format, ...) printf(TAG ": " format "\n", ## __VA_ARGS__)
#else
#define __apple_log_print(format, ...)
#endif
#ifndef LOGD
#define LOGD(format, ...) __apple_log_print(format, ## __VA_ARGS__)
#endif
#ifndef LOGI
#define LOGI(format, ...) __apple_log_print(format, ## __VA_ARGS__)
#endif
#ifndef LOGW
#define LOGW(format, ...) __apple_log_print(format, ## __VA_ARGS__)
#endif
#ifndef LOGE
#define LOGE(format, ...) __apple_log_print(format, ## __VA_ARGS__)
#endif
#ifndef LOGF
#define LOGF(format, ...) __apple_log_print(format, ## __VA_ARGS__)
#endif

#endif

#ifndef NDEBUG
#define _GL_ERROR(expression, file, line) expression; { \
    GLenum error = glGetError(); \
    if (error != GL_NO_ERROR) { \
        LOGD("error: [%X], file: [%s], line: [%d], expression: [%s]", error, file, line, #expression); \
    } \
}
#define GL_ERROR(expression) _GL_ERROR(expression, __FILE__, __LINE__)
#else
#define GL_ERROR(expression) expression
#endif

#ifndef NDEBUG
#define _EGL_ERROR(expression, file, line) \
    if (!(expression)) { \
        LOGD("error: [%X], file: [%s], line: [%d], expression: [%s]", eglGetError(), file, line, #expression); \
    }
#define EGL_ERROR(expression) _EGL_ERROR(expression, __FILE__, __LINE__)
#else
#define EGL_ERROR(expression) expression
#endif

#ifdef __cplusplus
#include "Resource.h"
#include "Renderer.h"
#include "Framebuffer.h"
#include "RenderFilter.h"
#include "RenderObject.h"
#include "RenderFactory.h"
#include "Texture.h"
#include "RenderContext.h"
#include "RenderThread.h"
#include "RenderLayer.h"
#include "VideoSource.h"
#include "source/RGB24VideoSource.h"
#include "source/RGBAVideoSource.h"
#include "source/YUV420PVideoSource.h"
#include "source/YUV420SPVideoSource.h"
#include "source/URLVideoSource.h"
#endif
