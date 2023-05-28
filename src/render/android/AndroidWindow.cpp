//
// Created by 熊朝伟 on 2020-03-20.
//

#include "defines.h"

OMP_RENDER_ANDROID_USING_NAMESPACE

RefPtr<RenderTarget> RenderContext::createWindow(const void *window) {
    return new AndroidWindow((EGLNativeWindowType)window);
}

AndroidWindow::AndroidWindow(ANativeWindow *window) : _window(window) {

}

EGLNativeWindowType AndroidWindow::handle() const {
    return _window;
}

vec2 AndroidWindow::size() const {
    assert(_window != nullptr);
    return vec2(ANativeWindow_getWidth(_window), ANativeWindow_getHeight(_window));
}

template <>
void RefPtr<ANativeWindow>::retain(ANativeWindow *value) {
    ANativeWindow_acquire(value);
}

template <>
bool RefPtr<ANativeWindow>::release(ANativeWindow *value) {
    ANativeWindow_release(value);
    return false;
}

template <>
void RefPtr<ANativeWindow>::destroy(ANativeWindow *value) {
}
