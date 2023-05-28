//
// Created by 熊朝伟 on 2020-03-19.
//

OMP_RENDER_EGL_NAMESPACE_BEGIN

class EGLFramebuffer;

class EGLRenderContext : public RenderContext {
    friend class EGLFramebuffer;
protected:
    EGLNativeDisplayType _nativeDisplay = 0;
    EGLConfig _eglConfig = 0;
    EGLDisplay _eglDisplay = 0;
    EGLContext _eglContext = 0;
public:
    EGLRenderContext(EGLNativeDisplayType nativeDisplay = EGL_DEFAULT_DISPLAY);

    void load() override;
    void unload() override;

    void makeCurrent() override;
    void render() override;
    void present() override;
    RefPtr<Framebuffer> createFramebuffer(const void *window) override;

    void destroySurface(EGLFramebuffer *framebuffer);

    static EGLConfig chooseConfig(EGLDisplay eglDisplay, int redSize, int greenSize, int blueSize, int alphaSize, int depthSize, int stencilSize, int renderType);
    static EGLContext createContext(EGLDisplay eglDisplay, EGLConfig eglConfig);
    static EGLSurface createSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLNativeWindowType window);
    static EGLSurface createSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, size_t width, size_t height);
};

OMP_RENDER_EGL_NAMESPACE_END
