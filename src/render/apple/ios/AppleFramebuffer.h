//
//  AppleFramebuffer.h
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

OMP_RENDER_APPLE_NAMESPACE_BEGIN

class AppleFramebuffer : public Framebuffer {
public:
    AppleFramebuffer();
    AppleFramebuffer(GLint width, GLint height);
    
    void bind(RefPtr<RenderContext> context) override;
    void unbind(RefPtr<RenderContext> context) override;
    
    GLint width() const override;
    GLint height() const override;
    
    virtual void present() = 0;
protected:
    GLint _width;
    GLint _height;
    GLuint _framebuffer = 0;
};

class AppleLayerFramebuffer : public AppleFramebuffer {
    GLuint _colorRenderbuffer = 0;
    GLuint _depthRenderbuffer = 0;
    CAEAGLLayer *_layer;
public:
    AppleLayerFramebuffer(CAEAGLLayer *layer);
    ~AppleLayerFramebuffer();
    
    void present() override;
};

class AppleLayerMultisamplingFramebuffer : public AppleFramebuffer {
    GLuint _colorRenderbuffer = 0;
    GLuint _depthRenderbuffer = 0;
    GLuint _resolveFramebuffer = 0;
    GLuint _resolveColorRenderbuffer = 0;
    CAEAGLLayer *_layer;
public:
    AppleLayerMultisamplingFramebuffer(CAEAGLLayer *layer);
    ~AppleLayerMultisamplingFramebuffer();
    
    void present() override;
};

// class AppleTextureFramebuffer : public AppleFramebuffer {
//     GLuint _depthRenderbuffer = 0;
//     GLuint _texture = 0;
// public:
//     AppleTextureFramebuffer(GLint width, GLint height);
//     ~AppleTextureFramebuffer();
    
//     GLuint texture() override;
//     void present() override;
// };

OMP_RENDER_APPLE_NAMESPACE_END
