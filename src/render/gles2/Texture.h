//
// Created by 熊朝伟 on 2020-03-21.
//

#pragma once

OMP_RENDER_GLES2_NAMESPACE_BEGIN

class Texture : public render::Texture {
    GLuint _texture;
    Texture(const Texture &) = delete;
    Texture(const Texture &&) = delete;
public:
    Texture();
    ~Texture();

    GLuint data() const override { return _texture; }

    void setImage(int32_t width, int32_t height, const uint8_t *pixels, const Texture::Type &type) override;
    void setImage(GLsizei width, GLsizei height, const uint8_t *pixels, GLenum format);
    void setSubImage(GLint offsetX, GLint offsetY, GLsizei width, GLsizei height, const uint8_t *pixels, GLenum format = GL_RGBA);
};

OMP_RENDER_GLES2_NAMESPACE_END
