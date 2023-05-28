//
// Created by 熊朝伟 on 2020-03-20.
//

OMP_RENDER_EGL_NAMESPACE_BEGIN

class EGLWindow : public gles2::Window {
protected:
public:
    virtual EGLNativeWindowType handle() const = 0;

    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
};

OMP_RENDER_EGL_NAMESPACE_END
