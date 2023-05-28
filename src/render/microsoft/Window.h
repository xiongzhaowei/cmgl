//
// Created by 熊朝伟 on 2020-04-30.
//

#pragma once

OMP_RENDER_MS_NAMESPACE_BEGIN

class OSWindow : public egl::EGLWindow {
    EGLNativeWindowType _window;
public:
    OSWindow(EGLNativeWindowType window);

    EGLNativeWindowType handle() const override;
    vec2 size() const override;
};

OMP_RENDER_MS_NAMESPACE_END
