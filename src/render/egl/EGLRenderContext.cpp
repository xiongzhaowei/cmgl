//
// Created by 熊朝伟 on 2020-03-19.
//

#include "defines.h"

OMP_RENDER_EGL_USING_NAMESPACE

EGLRenderContext::EGLRenderContext(EGLNativeDisplayType nativeDisplay) : _nativeDisplay(nativeDisplay) {
}

void EGLRenderContext::load() {
    if (_isEnabled) return;

    _eglDisplay = eglGetDisplay(_nativeDisplay);

    EGLint version[2];
    eglInitialize(_eglDisplay, &version[0], &version[1]);

    _eglConfig = chooseConfig(_eglDisplay, 8, 8, 8, 8, 16, 0, EGL_OPENGL_ES2_BIT);
    _eglContext = createContext(_eglDisplay, _eglConfig);

    eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, _eglContext);

    for (RefPtr<RenderSource> source : _renderSources) {
        source->load(this);
    }
    if (_renderTarget) _renderTarget->load(this);

    _isEnabled = true;
}

void EGLRenderContext::unload() {
    if (!_isEnabled) return;
    _isEnabled = false;

    if (_renderTarget) _renderTarget->unload(this);
    for (RefPtr<RenderSource> source : _renderSources) {
        source->unload(this);
    }
    _framebuffer = nullptr;
    eglMakeCurrent(_eglDisplay, nullptr, nullptr, nullptr);
    eglDestroyContext(_eglDisplay, _eglContext);

    eglTerminate(_eglDisplay);
    _eglDisplay = nullptr;
}

void EGLRenderContext::render() {
    assert(_renderTarget != nullptr);
    if (!_isEnabled) return;
    if (!_renderTarget) return;
    _renderTarget->startRender(this);
    for (RefPtr<RenderSource> source : _renderSources) {
        _renderTarget->render(this, source);
    }
    _renderTarget->finishRender(this);
}

void EGLRenderContext::present() {
    assert(_framebuffer != nullptr);
    assert(_framebuffer.as<EGLFramebuffer>() != nullptr);
    EGL_ERROR(eglSwapBuffers(_eglDisplay, _framebuffer.cast<EGLFramebuffer>()->_eglSurface));
}

RefPtr<Framebuffer> EGLRenderContext::createFramebuffer(const void *window) {
    return new EGLFramebuffer(createSurface(_eglDisplay, _eglConfig, (EGLNativeWindowType)window));
}

void EGLRenderContext::destroySurface(EGLFramebuffer *framebuffer) {
    eglDestroySurface(_eglDisplay, framebuffer->_eglSurface);
    framebuffer->_eglSurface = EGL_NO_SURFACE;
}

void EGLRenderContext::makeCurrent() {
    if (_isEnabled) {
        EGLSurface surface = _framebuffer ? _framebuffer.cast<EGLFramebuffer>()->_eglSurface : EGL_NO_SURFACE;
        eglMakeCurrent(_eglDisplay, surface, surface, _eglContext);
    }
}

EGLSurface EGLRenderContext::createSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLNativeWindowType window) {
    EGLint attributes[] = {EGL_NONE};
    return eglCreateWindowSurface(eglDisplay, eglConfig, window, attributes);
}

EGLSurface EGLRenderContext::createSurface(EGLDisplay eglDisplay, EGLConfig eglConfig, size_t width, size_t height) {
    EGLint attributes[] = {EGL_WIDTH, (int)width, EGL_HEIGHT, (int)height, EGL_NONE};
    return eglCreatePbufferSurface(eglDisplay, eglConfig, attributes);
}

EGLConfig EGLRenderContext::chooseConfig(EGLDisplay eglDisplay, int redSize, int greenSize, int blueSize, int alphaSize, int depthSize, int stencilSize, int renderType) {
    int configSpec[] = {
        EGL_RED_SIZE, redSize,
        EGL_GREEN_SIZE, greenSize,
        EGL_BLUE_SIZE, blueSize,
        EGL_ALPHA_SIZE, alphaSize,
        EGL_DEPTH_SIZE, depthSize,
        EGL_STENCIL_SIZE, stencilSize,
        EGL_RENDERABLE_TYPE, renderType,
        EGL_NONE,
    };

    EGLint numberOfConfigs;
    if (!eglChooseConfig(eglDisplay, configSpec, nullptr, 0, &numberOfConfigs)) {
        return nullptr;
    }

    if (numberOfConfigs <= 0) {
        return nullptr;
    }

    std::vector<EGLConfig> configs;// = new EGLConfig[numberOfConfigs];
    configs.resize(numberOfConfigs);
    if (!eglChooseConfig(eglDisplay, configSpec, configs.data(), numberOfConfigs, &numberOfConfigs)) {
        return nullptr;
    }

    for (EGLConfig config : configs) {
        int depth, stencil;
        eglGetConfigAttrib(eglDisplay, config, EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(eglDisplay, config, EGL_STENCIL_SIZE, &stencil);
        if ((depth >= depthSize) && (stencil >= stencilSize)) {
            int red, green, blue, alpha;
            eglGetConfigAttrib(eglDisplay, config, EGL_RED_SIZE, &red);
            eglGetConfigAttrib(eglDisplay, config, EGL_GREEN_SIZE, &green);
            eglGetConfigAttrib(eglDisplay, config, EGL_BLUE_SIZE, &blue);
            eglGetConfigAttrib(eglDisplay, config, EGL_ALPHA_SIZE, &alpha);
            if ((red == redSize) && (green == greenSize) && (blue == blueSize) && (alpha == alphaSize)) {
                return config;
            }
        }
    }

    return nullptr;
}

EGLContext EGLRenderContext::createContext(EGLDisplay eglDisplay, EGLConfig eglConfig) {
    int attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };
    return eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, attributes);
}
