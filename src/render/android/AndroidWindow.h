//
// Created by 熊朝伟 on 2020-03-20.
//

#pragma once

OMP_RENDER_ANDROID_NAMESPACE_BEGIN

class AndroidWindow : public egl::EGLWindow {
    RefPtr<ANativeWindow> _window;
public:
    AndroidWindow(ANativeWindow *window);

    EGLNativeWindowType handle() const override;
    vec2 size() const override;
};

OMP_RENDER_ANDROID_NAMESPACE_END

OMP_NAMESPACE_BEGIN

template <> void RefPtr<ANativeWindow>::retain(ANativeWindow *value);
template <> bool RefPtr<ANativeWindow>::release(ANativeWindow *value);
template <> void RefPtr<ANativeWindow>::destroy(ANativeWindow *value);

OMP_NAMESPACE_END
