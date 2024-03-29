﻿//
//  YUV420PVideoSource.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#include "defines.h"
#include "YUV420PVideoSource.h"

OMP_RENDER_USING_NAMESPACE

void YUV420PVideoSource::load(RefPtr<RenderContext> context) {
    _texture1 = context->createTexture();
    _texture2 = context->createTexture();
    _texture3 = context->createTexture();
}

void YUV420PVideoSource::unload(RefPtr<RenderContext> context) {
    _texture1 = nullptr;
    _texture2 = nullptr;
    _texture3 = nullptr;
}

void YUV420PVideoSource::clear(uint8_t red, uint8_t green, uint8_t blue) {
    std::lock_guard<std::mutex> lock(_lock);

    int32_t width = 16;
    int32_t height = 16;
    int32_t size = width * height;
    vec4 color = Renderer::kColorConversionBT601FullRange / vec4(red / 255.0, green / 255.0, blue / 255.0, 1);
    color *= 255;

    _pixels1.resize(size);
    memset(_pixels1.data(), color.r, size);

    size = (((size - 1) | 3) + 1) >> 2;

    _pixels2.resize(size);
    memset(_pixels2.data(), color.g, size);

    _pixels3.resize(size);
    memset(_pixels3.data(), color.b, size);

    _size = ivec2(width, height);
    _isNeedsUpdate = true;
}

void YUV420PVideoSource::update(const AVFrame *frame) {
    assert(frame != nullptr);

#define ALIGN(num, align) ((((num) - 1) | ((align) - 1)) + 1)
    std::lock_guard<std::mutex> lock(_lock);

    int32_t linesize = frame->linesize[0];
    int32_t width = frame->width;
    int32_t height = frame->height;
    int32_t stride = ALIGN(width, 4);
    int32_t size = stride * height;
    _size = ivec2(width, height);

    _pixels1.resize(size);
    for (int32_t y = 0; y < height; y++) {
        memcpy(_pixels1.data() + stride * y, frame->data[0] + linesize * y, width);
    }

    linesize = frame->linesize[1];
    width = ALIGN(width, 2) >> 1;
    height = ALIGN(height, 2) >> 1;
    stride = ALIGN(stride >> 1, 4);
    size = stride * height;

    _pixels2.resize(size);
    for (int32_t y = 0; y < height; y++) {
        memcpy(_pixels2.data() + stride * y, frame->data[1] + linesize * y, width);
    }

    linesize = frame->linesize[2];

    _pixels3.resize(size);
    for (int32_t y = 0; y < height; y++) {
        memcpy(_pixels3.data() + stride * y, frame->data[2] + linesize * y, width);
    }

    _isNeedsUpdate = true;
#undef ALIGN
}

bool YUV420PVideoSource::support(const Type &type) {
    return YUV420P == type;
}

void YUV420PVideoSource::draw(
    RefPtr<RenderContext> context,
    RefPtr<Framebuffer> framebuffer,
    const mat4 &globalMatrix,
    const mat4 &localMatrix,
    const mat4 &clipMatrix,
    const vec2 &size,
    float alpha
) {
    if (_size.x * _size.y == 0) return;
    if (_isNeedsUpdate) {
        std::lock_guard<std::mutex> lock(_lock);

        _texture1->setImage(_size.x, _size.y, _pixels1.data(), Texture::Alpha);
        _texture2->setImage((_size.x - 1) / 2 + 1, (_size.y - 1) / 2 + 1, _pixels2.data(), Texture::Alpha);
        _texture3->setImage((_size.x - 1) / 2 + 1, (_size.y - 1) / 2 + 1, _pixels3.data(), Texture::Alpha);
        _isNeedsUpdate = false;
    }

    context->draw(Renderer::YUV420P, framebuffer, globalMatrix, localMatrix, clipMatrix, Renderer::kColorConversionBT709, size, alpha, _texture1->data(), _texture2->data(), _texture3->data());
}
