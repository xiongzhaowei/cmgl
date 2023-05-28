//
// Created by 熊朝伟 on 2020-03-20.
//

#pragma once

OMP_RENDER_APPLE_NAMESPACE_BEGIN

class AppleWindow : public egl::EGLWindow {
    RefPtr<CALayer> _window;
public:
    AppleWindow(CALayer *window);

    vec2 size() const override;

protected:
    EGLNativeWindowType window() override;
};

OMP_RENDER_APPLE_NAMESPACE_END

OMP_NAMESPACE_BEGIN

template <> void RefPtr<CALayer>::retain(CALayer *value);
template <> bool RefPtr<CALayer>::release(CALayer *value);
template <> void RefPtr<CALayer>::destroy(CALayer *value);

OMP_NAMESPACE_END
