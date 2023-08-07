//
// Created by 熊朝伟 on 2020-03-16.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

void DataSource::render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) {
    if (!visible()) return;
    draw(context, framebuffer, globalMatrix, identity<mat4>(), identity<mat4>(), vec2(framebuffer->width(), framebuffer->height()), 1);
}

void RenderFilter::load(RefPtr<RenderContext> context) {
}

void RenderFilter::unload(RefPtr<RenderContext> context) {
}

vec2 RenderFilter::size() const {
    return _size;
}

void RenderFilter::startRender(RefPtr<RenderContext> context) {
}

void RenderFilter::finishRender(RefPtr<RenderContext> context) {
}

void RenderFilter::render(RefPtr<RenderContext> context, RefPtr<RenderSource> source) {
    assert(source != nullptr);
    bool rebuild = false;
    _size = source->size();
    do {
        if (!_framebuffer) {
            rebuild = true;
            break;
        }

        if (_size.x != _framebuffer->width()) {
            rebuild = true;
            break;
        }
        
        if (_size.y != _framebuffer->height()) {
            rebuild = true;
            break;
        }
    } while(false);

    if (rebuild) {
        _framebuffer = context->createFramebuffer(_size.x, _size.y);
    }

    source->render(context, _framebuffer, glm::ortho<float>(0, _size.x, _size.y, 0));
}

void RenderFilter::render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) {
    mat4 matrix = identity<mat4>();
    context->draw(Renderer::RGBA, framebuffer, globalMatrix, matrix, matrix, Renderer::kColorConversionNone, _size, 1, _framebuffer->texture());
}
