//
// Created by 熊朝伟 on 2020-03-16.
//

#include "defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

TextureFramebuffer::TextureFramebuffer(GLint width, GLint height) : _width(width), _height(height) {
    GL_ERROR(glGenFramebuffers(1, &_framebuffer));
    GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer));

    GL_ERROR(glGenTextures(1, &_texture));
    GL_ERROR(glBindTexture(GL_TEXTURE_2D, _texture));
    GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
    GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0));

    GL_ERROR(glGenRenderbuffers(1, &_depthRenderbuffer));
    GL_ERROR(glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer));
    GL_ERROR(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _width, _height));
    GL_ERROR(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer));
}

void TextureFramebuffer::bind(RefPtr<RenderContext> context) {
    GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer));
}

void TextureFramebuffer::unbind(RefPtr<RenderContext> context) {
    GL_ERROR(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLint TextureFramebuffer::width() const {
    return _width;
}

GLint TextureFramebuffer::height() const {
    return _height;
}

TextureFramebuffer::~TextureFramebuffer() {
    _width = 0;
    _height = 0;
    
    if (_framebuffer) {
        GL_ERROR(glDeleteFramebuffers(1, &_framebuffer));
        _framebuffer = 0;
    }
    
    if (_texture) {
        GL_ERROR(glDeleteTextures(1, &_texture));
        _texture = 0;
    }
    
    if (_depthRenderbuffer) {
        GL_ERROR(glDeleteRenderbuffers(1, &_depthRenderbuffer));
        _depthRenderbuffer = 0;
    }
}

GLuint TextureFramebuffer::texture() {
    return _texture;
}
