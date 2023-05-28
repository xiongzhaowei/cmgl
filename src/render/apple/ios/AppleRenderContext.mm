//
//  AppleRenderContext.mm
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

#include "../defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

void AppleRenderContext::load() {
    if (_isEnabled) return;

    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    [EAGLContext setCurrentContext:_context];
    
    for (RefPtr<RenderSource> source : _renderSources) {
        source->load(this);
    }
    if (_renderTarget) _renderTarget->load(this);

    _isEnabled = true;
}

void AppleRenderContext::unload() {
    if (!_isEnabled) return;
    _isEnabled = false;

    if (_renderTarget) _renderTarget->unload(this);
    for (RefPtr<RenderSource> source : _renderSources) {
        source->unload(this);
    }
    _framebuffer = nullptr;
    
    [EAGLContext setCurrentContext:nullptr];
    _context = nullptr;
}

void AppleRenderContext::render() {
    assert(_renderTarget != nullptr);
    if (!_isEnabled) return;
    _renderTarget->startRender(this);
    for (RefPtr<RenderSource> source : _renderSources) {
        _renderTarget->render(this, source);
    }
    _renderTarget->finishRender(this);
}

void AppleRenderContext::present() {
    if (!_framebuffer) return;

    assert(_framebuffer.as<AppleFramebuffer>());
    _framebuffer.cast<AppleFramebuffer>()->present();
}

RefPtr<Framebuffer> AppleRenderContext::createFramebuffer(const void *window) {
    return new AppleLayerFramebuffer((__bridge CAEAGLLayer *)window);
}

void AppleRenderContext::setFramebuffer(RefPtr<Framebuffer> framebuffer) {
    if (framebuffer == _framebuffer) return;

    if (framebuffer) {
        assert(framebuffer.as<AppleFramebuffer>());
        framebuffer.cast<AppleFramebuffer>()->bind(this);
    }
    RenderContext::setFramebuffer(framebuffer);
}

void AppleRenderContext::makeCurrent() {
    if (_isEnabled) {
        [EAGLContext setCurrentContext:_context];
    }
}

template <>
void RefPtr<EAGLContext>::retain(EAGLContext *value) {
    CFRetain((CFTypeRef)value);
}

template <>
bool RefPtr<EAGLContext>::release(EAGLContext *value) {
    CFRelease((CFTypeRef)value);
    return false;
}

template <>
void RefPtr<EAGLContext>::destroy(EAGLContext *value) {
}
