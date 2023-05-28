//
// Created by 熊朝伟 on 2020-03-19.
//

#pragma once

#include "../defines.h"

#define OMP_RENDER_EGL_NAMESPACE_BEGIN    OMP_RENDER_NAMESPACE_BEGIN namespace egl {
#define OMP_RENDER_EGL_NAMESPACE_END      } OMP_RENDER_NAMESPACE_END
#define OMP_RENDER_EGL_USING_NAMESPACE    OMP_RENDER_USING_NAMESPACE using namespace OMP_RENDER_NAMESPACE_PREFIX egl;

#ifdef __ANDROID__
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <EGL/egl.h>
#elif TARGET_OS_IPHONE
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
#elif TARGET_OS_OSX
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <EGL/egl.h>
#endif

#include "../gles2/defines.h"
#include "EGLWindow.h"
#include "EGLRenderContext.h"
#include "EGLFramebuffer.h"
