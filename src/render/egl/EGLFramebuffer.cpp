//
// Created by 熊朝伟 on 2020-03-20.
//

#include "defines.h"

OMP_RENDER_EGL_USING_NAMESPACE

EGLFramebuffer::EGLFramebuffer(EGLSurface eglSurface) : _eglSurface(eglSurface) {

}

EGLFramebuffer::~EGLFramebuffer() {
    assert(_eglSurface == EGL_NO_SURFACE);
}

int32_t EGLFramebuffer::width() const {
    int32_t result = 0;
    EGL_ERROR(eglQuerySurface(eglGetCurrentDisplay(), _eglSurface, EGL_WIDTH, &result));
    return result;
}

int32_t EGLFramebuffer::height() const {
    int32_t result = 0;
    EGL_ERROR(eglQuerySurface(eglGetCurrentDisplay(), _eglSurface, EGL_HEIGHT, &result));
    return result;
}

void EGLFramebuffer::bind(RefPtr<RenderContext> context) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    EGLDisplay eglDisplay = context.cast<EGLRenderContext>()->_eglDisplay;
    EGLContext eglContext = context.cast<EGLRenderContext>()->_eglContext;
    EGL_ERROR(eglMakeCurrent(eglDisplay, _eglSurface, _eglSurface, eglContext));
}

void EGLFramebuffer::unbind(RefPtr<RenderContext> context) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    EGLDisplay eglDisplay = context.cast<EGLRenderContext>()->_eglDisplay;
    EGLContext eglContext = context.cast<EGLRenderContext>()->_eglContext;
    EGL_ERROR(eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext));
}
