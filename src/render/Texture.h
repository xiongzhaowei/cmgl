//
// Created by 熊朝伟 on 2020-03-21.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct Texture : public Object {
    typedef enum {
        None,
        RGBA,
        RGB24,
        Alpha,
        Luminance,
        LuminanceAlpha,
    } Type;

    virtual uint32_t data() const = 0;
    virtual void setImage(int32_t width, int32_t height, const uint8_t *pixels, const Type &type) = 0;
};

OMP_RENDER_NAMESPACE_END
