//
//  YUV420SPVideoSource.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#include "defines.h"
#include "YUV420SPVideoSource.h"

OMP_RENDER_USING_NAMESPACE

void YUV420SPVideoSource::load(RefPtr<RenderContext> context) {
    _texture1 = context->createTexture();
    _texture2 = context->createTexture();
}

void YUV420SPVideoSource::unload(RefPtr<RenderContext> context) {
    _texture1 = nullptr;
    _texture2 = nullptr;
}

void YUV420SPVideoSource::update(const AVFrame *frame) {
    assert(frame != nullptr);

    std::lock_guard<std::mutex> lock(_lock);

    int32_t width = frame->width;
    int32_t height = frame->height;
    int32_t size = width * height;

    _pixels1.resize(size);
    memcpy(_pixels1.data(), frame->data[0], size);

    size = (((size - 1) | 3) + 1) >> 2; // 2 bytes align and divide 2.

    _pixels2.resize(size * 2);
    memcpy(_pixels2.data(), frame->data[1], size * 2);

    _size = ivec2(width, height);
    _isNeedsUpdate = true;
}

bool YUV420SPVideoSource::support(const Type &type) {
    return YUV420SP == type;
}

void YUV420SPVideoSource::draw(
    RefPtr<RenderContext> context,
    RefPtr<Framebuffer> framebuffer,
    const mat4 &globalMatrix,
    const mat4 &localMatrix,
    const mat4 &clipMatrix,
    const vec2 &size,
    float alpha
) {
    if (_isNeedsUpdate) {
        std::lock_guard<std::mutex> lock(_lock);

        _texture1->setImage(_size.x, _size.y, _pixels1.data(), Texture::Alpha);
        _texture2->setImage((_size.x - 1) / 2 + 1, (_size.y - 1) / 2 + 1, _pixels2.data(), Texture::LuminanceAlpha);
        _isNeedsUpdate = false;
    }

    context->draw(Renderer::YUV420SP, framebuffer, globalMatrix, localMatrix, clipMatrix, Renderer::kColorConversionBT601FullRange, size, alpha, _texture1->data(), _texture2->data());
}
