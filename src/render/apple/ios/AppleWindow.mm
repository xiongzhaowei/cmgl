//
//  AppleWindow.mm
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

#include "../defines.h"
#include <CoreFoundation/CoreFoundation.h>
#include <QuartzCore/QuartzCore.h>

OMP_RENDER_APPLE_USING_NAMESPACE

AppleWindow::AppleWindow(CALayer *window) : _window(window) {
    assert([window isKindOfClass:[CAEAGLLayer class]]);
}

vec2 AppleWindow::size() const {
    assert(_window != nullptr);
    CGSize size = [_window bounds].size;
    return vec2(size.width, size.height);
}

void AppleWindow::load(RefPtr<RenderContext> context) {
    _framebuffer = context->createFramebuffer((__bridge CFTypeRef)(_window.value()));
}

void AppleWindow::unload(RefPtr<RenderContext> context) {
    if (_framebuffer) {
        context->setFramebuffer(nullptr);
        _framebuffer = nullptr;
    }
}

template <>
void RefPtr<CALayer>::retain(CALayer *value) {
    CFRetain((__bridge CFTypeRef)value);
}

template <>
bool RefPtr<CALayer>::release(CALayer *value) {
    CFRelease((__bridge CFTypeRef)value);
    return false;
}

template <>
void RefPtr<CALayer>::destroy(CALayer *value) {
}
