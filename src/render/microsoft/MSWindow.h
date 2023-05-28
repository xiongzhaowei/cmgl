//
// Created by 熊朝伟 on 2020-04-30.
//

#pragma once

OMP_RENDER_MS_NAMESPACE_BEGIN

class MSWindow : public egl::EGLWindow {
    EGLNativeWindowType _window;
public:
    MSWindow(EGLNativeWindowType window);

    vec2 size() const override;

protected:
    EGLNativeWindowType window() override;
};

OMP_RENDER_MS_NAMESPACE_END
