//
//  RGB24VideoSource.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#ifdef _WIN32
#include <Windows.h>
#include <gdiplus.h>
#endif // _WIN32
#include "defines.h"
#include "RGB24VideoSource.h"


OMP_RENDER_USING_NAMESPACE

void RGB24VideoSource::load(RefPtr<RenderContext> context) {
    _texture = context->createTexture();
}

void RGB24VideoSource::unload(RefPtr<RenderContext> context) {
    _texture = nullptr;
}

void RGB24VideoSource::update(const AVFrame *frame) {
    assert(frame != nullptr);

    std::lock_guard<std::mutex> lock(_lock);

    int32_t width = frame->width;
    int32_t height = frame->height;
    int32_t size = width * height * 3;

    _pixels.resize(size);
    memcpy(_pixels.data(), frame->data[0], size);

    _size = ivec2(width, height);
    _isNeedsUpdate = true;
}

#ifdef _WIN32
void RGB24VideoSource::update(Gdiplus::Bitmap* bitmap) {
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

bool RGB24VideoSource::support(const Type &type) {
    return RGB24 == type;
}

void RGB24VideoSource::draw(
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

        _texture->setImage(_size.x, _size.y, _pixels.data(), Texture::RGB24);
        _isNeedsUpdate = false;
    }

    context->draw(Renderer::RGBA, framebuffer, globalMatrix, localMatrix, clipMatrix, Renderer::kColorConversionNone, size, alpha, _texture->data());
}
