//
// Created by 熊朝伟 on 2020-03-21.
//

#include "defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

gles2::Texture::Texture() {
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

gles2::Texture::~Texture() {
    if (_texture) {
        glDeleteTextures(1, &_texture);
        _texture = 0;
    }
}

void gles2::Texture::setImage(int32_t width, int32_t height, const uint8_t *pixels, const Texture::Type &type) {
    GLenum format;
    switch (type) {
    case Texture::RGBA:
        format = GL_RGBA;
        break;
    case Texture::RGB24:
        format = GL_RGB;
        break;
    case Texture::Alpha:
        format = GL_ALPHA;
        break;
    case Texture::Luminance:
        format = GL_LUMINANCE;
        break;
    case Texture::LuminanceAlpha:
        format = GL_LUMINANCE_ALPHA;
        break;
    default:
        assert(false);
        break;
    }
    setImage(width, height, pixels, format);
}

void gles2::Texture::setImage(GLsizei width, GLsizei height, const uint8_t *pixels, GLenum format) {
    glBindTexture(GL_TEXTURE_2D, _texture);
    GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels));
}

void gles2::Texture::setSubImage(GLint offsetX, GLint offsetY, GLsizei width, GLsizei height, const uint8_t *pixels, GLenum format) {
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, offsetX, offsetY, width, height, format, GL_UNSIGNED_BYTE, pixels);
}
