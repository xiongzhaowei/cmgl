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
    width = ALIGN(width, 2);
    height = ALIGN(height, 2) >> 1;
    size = stride * height;

    _pixels2.resize(size);
    for (int32_t y = 0; y < height; y++) {
        memcpy(_pixels2.data() + stride * y, frame->data[1] + linesize * y, width);
    }

    _isNeedsUpdate = true;
#undef ALIGN
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

    mat4 colorConversion = Renderer::kColorConversionBT709;
#ifdef _WIN32
    for (int i = 0; i < 4; i++) {
        float value = colorConversion[i][0];
        colorConversion[i][0] = colorConversion[i][2];
        colorConversion[i][2] = value;
    }
#endif // _WIN32
    context->draw(Renderer::YUV420SP, framebuffer, globalMatrix, localMatrix, clipMatrix, colorConversion, size, alpha, _texture1->data(), _texture2->data());
}
