//
// Created by 熊朝伟 on 2020-03-20.
//

#include "defines.h"

OMP_RENDER_EGL_USING_NAMESPACE

void EGLWindow::load(RefPtr<RenderContext> context) {
    _framebuffer = context->createFramebuffer(handle());
}

void EGLWindow::unload(RefPtr<RenderContext> context) {
    if (_framebuffer) {
        context->setFramebuffer(nullptr);
        context.cast<EGLRenderContext>()->destroySurface(_framebuffer.cast<EGLFramebuffer>());
        _framebuffer = nullptr;
    }
}
