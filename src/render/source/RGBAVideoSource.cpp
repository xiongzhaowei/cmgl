//
//  RGBAVideoSource.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#ifdef _WIN32
#include <Windows.h>
#include <gdiplus.h>
#endif // _WIN32
#include "defines.h"
#include "RGBAVideoSource.h"

OMP_RENDER_USING_NAMESPACE

void RGBAVideoSource::load(RefPtr<RenderContext> context) {
    _texture = context->createTexture();
}

void RGBAVideoSource::unload(RefPtr<RenderContext> context) {
    _texture = nullptr;
}

void RGBAVideoSource::update(const AVFrame *frame) {
    assert(frame != nullptr);

    int32_t width = frame->width;
    int32_t height = frame->height;
    int32_t size = width * height * 4;

    _pixels.resize(size);
    memcpy(_pixels.data(), frame->data[0], size);

    _size = ivec2(width, height);
    _isNeedsUpdate = true;
}

bool RGBAVideoSource::support(const Type &type) {
    return RGBA == type;
}

void RGBAVideoSource::update(int width, int height, const void *pixels) {
    std::lock_guard<std::mutex> lock(_lock);

    _pixels.resize(width * height * 4);
    memcpy(_pixels.data(), pixels, width * height * 4);
    _size = ivec2(width, height);
    _isNeedsUpdate = true;
}

#ifdef _WIN32
void RGBAVideoSource::update(Gdiplus::Bitmap* bitmap) {
    assert(bitmap != nullptr);

    std::lock_guard<std::mutex> lock(_lock);

    Gdiplus::PixelFormat format = PixelFormat32bppARGB;
    int32_t sizeOfColor = 4;

    int32_t width = bitmap->GetWidth();
    int32_t height = bitmap->GetHeight();
    int32_t size = width * height * sizeOfColor;
    Gdiplus::Rect rect(0, 0, width, height);

    Gdiplus::BitmapData bitmapData = { 0 };
    if (bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, format, &bitmapData) == Gdiplus::Ok) {
        _pixels.resize(size);
        uint8_t* begin = (uint8_t*)bitmapData.Scan0;
        uint8_t* end = (uint8_t*)bitmapData.Scan0 + bitmapData.Stride * height;
        int32_t step = bitmapData.Stride;
        int32_t line = width * sizeOfColor;

        for (uint8_t* src = begin, *dst = _pixels.data(); src != end; src += step, dst += line) {
            memcpy(dst, src, line);
        }

        _size = ivec2(width, height);
        _isNeedsUpdate = true;

        bitmap->UnlockBits(&bitmapData);
    }
}
#endif // _WIN32

void RGBAVideoSource::draw(
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

        _texture->setImage(_size.x, _size.y, _pixels.data(), Texture::RGBA);
        _pixels = std::vector<uint8_t>();
        _isNeedsUpdate = false;
    }

    context->draw(Renderer::RGBA, framebuffer, globalMatrix, localMatrix, clipMatrix, Renderer::kColorConversionExchangeRedAndBlue, size, alpha, _texture->data());
}

RefPtr<Texture> RGBAVideoSource::texture() const {
    return _texture;
}
