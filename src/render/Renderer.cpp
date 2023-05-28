//
//  Renderer.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/14.
//

#include "gles2/defines.h"

OMP_RENDER_USING_NAMESPACE

const mat4 Renderer::kColorConversionNone = mat4(
    1.0,    0.0,    0.0,    0.0,
    0.0,    1.0,    0.0,    0.0,
    0.0,    0.0,    1.0,    0.0,
    0.0,    0.0,    0.0,    1.0
);

// BT.709, which is the standard for HDTV.
const mat4 Renderer::kColorConversionBT709 = mat4(
    1.164,  1.164,  1.164,  0.0,
    0.0,   -0.213,  2.112,  0.0,
    1.793, -0.533,  0.0,    0.0,
    0.0,    0.0,    0.0,    1.0
) * glm::translate(identity<mat4>(), vec3(-16.0 / 255.0, -0.5, -0.5));

// BT.601, which is the standard for SDTV.
const mat4 Renderer::kColorConversionBT601 = mat4(
    1.164,  1.164,  1.164,  0.0,
    0.0,   -0.392,  2.017,  0.0,
    1.596, -0.813,  0.0,    0.0,
    0.0,    0.0,    0.0,    1.0
) * glm::translate(identity<mat4>(), vec3(-16.0 / 255.0, -0.5, -0.5));

// BT.601 full range
const mat4 Renderer::kColorConversionBT601FullRange = mat4(
    1.0,    1.0,    1.0,    0.0,
    0.0,   -0.343,  1.765,  0.0,
    1.4,   -0.711,  0.0,    0.0,
    0.0,    0.0,    0.0,    1.0
) * glm::translate(identity<mat4>(), vec3(-16.0 / 255.0, -0.5, -0.5));

RefPtr<Renderer> Renderer::createRenderer(Name name) {
    switch (name) {
    case None:
        return nullptr;
    case RGBA:
        return new gles2::RGBARenderer;
    case YUV420P:
        return new gles2::YUV420PRenderer;
    case YUV420SP:
        return new gles2::YUV420SPRenderer;
    default:
        assert(false);
        return nullptr;
    }
}
