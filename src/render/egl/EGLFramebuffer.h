//
// Created by 熊朝伟 on 2020-03-20.
//

OMP_RENDER_EGL_NAMESPACE_BEGIN

class EGLFramebuffer : public Framebuffer {
protected:
    friend class EGLRenderContext;
    EGLSurface _eglSurface;
public:
    EGLFramebuffer(EGLSurface eglSurface);
    ~EGLFramebuffer();

    void bind(RefPtr<RenderContext> context) override;
    void unbind(RefPtr<RenderContext> context) override;

    int32_t width() const override;
    int32_t height() const override;

    EGLSurface surface() const { return _eglSurface; }
};

OMP_RENDER_EGL_NAMESPACE_END
