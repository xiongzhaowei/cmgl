//
//  defines.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/23.
//

#pragma once

#include "../defines.h"

#define OMP_RENDER_GLES2_NAMESPACE_BEGIN    OMP_RENDER_NAMESPACE_BEGIN namespace gles2 {
#define OMP_RENDER_GLES2_NAMESPACE_END      } OMP_RENDER_NAMESPACE_END
#define OMP_RENDER_GLES2_USING_NAMESPACE    OMP_RENDER_USING_NAMESPACE using namespace OMP_RENDER_NAMESPACE_PREFIX gles2;

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
#elif defined(_WIN32)
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <EGL/egl.h>
#endif

#ifdef __cplusplus
#include "Program.h"
#include "Texture.h"
#include "Window.h"
#include "TextureFramebuffer.h"
#include "renderer/RGBARenderer.h"
#include "renderer/YUV420PRenderer.h"
#include "renderer/YUV420SPRenderer.h"

#include "TestRenderObject.h"
#endif
