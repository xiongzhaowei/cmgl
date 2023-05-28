//
// Created by 熊朝伟 on 2020-04-30.
//

#include "defines.h"

OMP_RENDER_MS_USING_NAMESPACE

RefPtr<RenderTarget> RenderContext::createWindow(const void *window) {
    return new MSWindow((EGLNativeWindowType)window);
}

MSWindow::MSWindow(EGLNativeWindowType window) : _window(window) {

}

vec2 MSWindow::size() const {
    assert(_window != nullptr);
    RECT rect = { 0 };
    if (GetClientRect(_window, &rect)) {
        return vec2(rect.right - rect.left, rect.bottom - rect.top);
    }
    return vec2(0.0f, 0.0f);
}

EGLNativeWindowType MSWindow::window() {
    return _window;
}
