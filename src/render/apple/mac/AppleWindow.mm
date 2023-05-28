//
// Created by 熊朝伟 on 2020-03-20.
//

#include "../defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

AppleWindow::AppleWindow(CALayer *window) : _window(window) {
    assert([window isKindOfClass:[CAOpenGLLayer class]]);
}

vec2 AppleWindow::size() const {
    assert(_window != nullptr);
    CGSize size = [_window bounds].size;
    return vec2(size.width, size.height);
}

EGLNativeWindowType AppleWindow::window() {
    return (__bridge EGLNativeWindowType)_window.value();
}

template <>
void RefPtr<CALayer>::retain(CALayer *value) {
    CFRetain((CFTypeRef)value);
}

template <>
bool RefPtr<CALayer>::release(CALayer *value) {
    CFRelease((CFTypeRef)value);
    return false;
}

template <>
void RefPtr<CALayer>::destroy(CALayer *value) {
}
