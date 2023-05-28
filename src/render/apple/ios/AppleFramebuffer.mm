//
//  AppleFramebuffer.mm
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

#include "../defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

AppleFramebuffer::AppleFramebuffer() : _width(0), _height(0) {
    
}

AppleFramebuffer::AppleFramebuffer(GLint width, GLint height) : _width(width), _height(height) {
    
}

void AppleFramebuffer::bind(RefPtr<RenderContext> context) {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}

void AppleFramebuffer::unbind(RefPtr<RenderContext> context) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint AppleFramebuffer::width() const {
    return _width;
}

GLint AppleFramebuffer::height() const {
    return _height;
}

AppleLayerFramebuffer::AppleLayerFramebuffer(CAEAGLLayer *layer) : _layer(layer) {
    layer.opaque = NO;
    layer.drawableProperties = @{
        kEAGLDrawablePropertyRetainedBacking: @YES,
        kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8,
    };

    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [[EAGLContext currentContext] renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_height);

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _width, _height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
}

AppleLayerFramebuffer::~AppleLayerFramebuffer() {
    _width = 0;
    _height = 0;
    
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    if (_colorRenderbuffer) {
        glDeleteRenderbuffers(1, &_colorRenderbuffer);
        _colorRenderbuffer = 0;
    }
    
    if (_depthRenderbuffer) {
        glDeleteRenderbuffers(1, &_depthRenderbuffer);
        _depthRenderbuffer = 0;
    }
}

void AppleLayerFramebuffer::present() {
    const GLenum discards[]  = { GL_DEPTH_ATTACHMENT };
    glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, discards);
    
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];
}

AppleLayerMultisamplingFramebuffer::AppleLayerMultisamplingFramebuffer(CAEAGLLayer *layer) {
    layer.opaque = NO;
    layer.drawableProperties = @{
        kEAGLDrawablePropertyRetainedBacking: @YES,
        kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8,
    };

    glGenFramebuffers(1, &_resolveFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _resolveFramebuffer);

    glGenRenderbuffers(1, &_resolveColorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _resolveColorRenderbuffer);
    [[EAGLContext currentContext] renderbufferStorage:GL_RENDERBUFFER fromDrawable:_layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _resolveColorRenderbuffer);

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_height);

    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_RGBA8_OES, _width, _height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, _width, _height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
}

AppleLayerMultisamplingFramebuffer::~AppleLayerMultisamplingFramebuffer() {
    _width = 0;
    _height = 0;
    
    if (_resolveFramebuffer) {
        glDeleteFramebuffers(1, &_resolveFramebuffer);
        _resolveFramebuffer = 0;
    }
    
    if (_resolveColorRenderbuffer) {
        glDeleteRenderbuffers(1, &_resolveColorRenderbuffer);
        _resolveColorRenderbuffer = 0;
    }
    
    if (_framebuffer) {
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = 0;
    }
    
    if (_colorRenderbuffer) {
        glDeleteRenderbuffers(1, &_colorRenderbuffer);
        _colorRenderbuffer = 0;
    }
    
    if (_depthRenderbuffer) {
        glDeleteRenderbuffers(1, &_depthRenderbuffer);
        _depthRenderbuffer = 0;
    }
}

void AppleLayerMultisamplingFramebuffer::present() {
    // 丢弃采样帧缓冲的颜色缓冲和深度缓冲
    const GLenum discards [] = { GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT };
    glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE, 2, discards);
    
    // 解析采样帧缓冲
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, _resolveFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, _framebuffer);
    glResolveMultisampleFramebufferAPPLE();
    
    // 渲染
    glBindRenderbuffer(GL_RENDERBUFFER, _resolveColorRenderbuffer);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];
}

// AppleTextureFramebuffer::AppleTextureFramebuffer(GLint width, GLint height) : AppleFramebuffer(width, height) {
//     glGenFramebuffers(1, &_framebuffer);
//     glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

//     glGenTextures(1, &_texture);
//     glBindTexture(GL_TEXTURE_2D, _texture);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

//     glGenRenderbuffers(1, &_depthRenderbuffer);
//     glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _width, _height);
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
// }

// AppleTextureFramebuffer::~AppleTextureFramebuffer() {
//     _width = 0;
//     _height = 0;
    
//     if (_framebuffer) {
//         glDeleteFramebuffers(1, &_framebuffer);
//         _framebuffer = 0;
//     }
    
//     if (_texture) {
//         glDeleteTextures(1, &_texture);
//         _texture = 0;
//     }
    
//     if (_depthRenderbuffer) {
//         glDeleteRenderbuffers(1, &_depthRenderbuffer);
//         _depthRenderbuffer = 0;
//     }
// }

// void AppleTextureFramebuffer::present() {
// }
