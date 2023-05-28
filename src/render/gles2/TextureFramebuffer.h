//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_GLES2_NAMESPACE_BEGIN

class TextureFramebuffer : public Framebuffer {
    GLint _width;
    GLint _height;
    GLuint _framebuffer = 0;
    GLuint _depthRenderbuffer = 0;
    GLuint _texture = 0;
public:
    TextureFramebuffer(GLint width, GLint height);
    ~TextureFramebuffer();

    void bind(RefPtr<RenderContext> context) override;
    void unbind(RefPtr<RenderContext> context) override;

    GLint width() const override;
    GLint height() const override;
    
    GLuint texture() override;
};

OMP_RENDER_GLES2_NAMESPACE_END
