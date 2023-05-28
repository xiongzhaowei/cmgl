//
// Created by 熊朝伟 on 2020-03-16.
//

#include "defines.h"
#include "gles2/defines.h"

OMP_RENDER_USING_NAMESPACE

RefPtr<Framebuffer> RenderContext::framebuffer() {
    return _framebuffer;
}

void RenderContext::setFramebuffer(RefPtr<Framebuffer> framebuffer) {
    if (framebuffer) {
        framebuffer->bind(this);
    }
    _framebuffer = framebuffer;
}

void RenderContext::load() {
}

void RenderContext::unload() {
}

RefPtr<RenderSource> RenderContext::source(int32_t index) {
    return _renderSources[index];
}

void RenderContext::addSource(RefPtr<RenderSource> source) {
    if (_isEnabled) source->load(this);
    _renderSources.push_back(source);
}

void RenderContext::removeSource(RefPtr<RenderSource> source) {
    for (auto it = _renderSources.begin(); it != _renderSources.end(); it++) {
        if (source == *it) {
            _renderSources.erase(it);
            if (_isEnabled) source->unload(this);
            break;
        }
    }
}

size_t RenderContext::numberOfSources() const {
    return _renderSources.size();
}

RefPtr<RenderTarget> RenderContext::target() {
    return _renderTarget;
}

void RenderContext::setTarget(RefPtr<RenderTarget> target) {
    if (_renderTarget == target) return;

    if (_isEnabled && _renderTarget) {
        _renderTarget->unload(this);
    }
    _renderTarget = target;
    if (_isEnabled && _renderTarget) {
        _renderTarget->load(this);
    }
}

void RenderContext::notifySourceChanged() {
    if (_sourceChangedCallback) {
        _sourceChangedCallback(this);
    }
}

void RenderContext::onSourceChanged(std::function<void(RenderContext* context)> callback) {
    _sourceChangedCallback = callback;
}

void RenderContext::draw(
    Renderer::Name name,
    RefPtr<Framebuffer> framebuffer,
    const mat4 &globalMatrix,
    const mat4 &localMatrix,
    const mat4 &clipMatrix,
    const mat4 &colorConversion,
    const vec2 &size,
    GLfloat alpha,
    GLuint texture, ...
) {
    assert(_isEnabled);
    if (name == Renderer::None) return;

    RefPtr<Renderer> renderer;
    auto it = _renderers.find(name);
    if (it != _renderers.end()) {
        renderer = it->second;
    }

    if (!renderer) {
        renderer = Renderer::createRenderer(name);
        renderer->load(this);
        _renderers[name] = renderer;
    }
    
    va_list list;
    va_start(list, texture);
    renderer->draw(this, framebuffer, globalMatrix, localMatrix, clipMatrix, colorConversion, size, alpha, texture, list);
    va_end(list);
}

RefPtr<Framebuffer> RenderContext::createFramebuffer(GLint width, GLint height) {
    return new gles2::TextureFramebuffer(width, height);
}

RefPtr<Texture> RenderContext::createTexture() {
    return new gles2::Texture;
}
