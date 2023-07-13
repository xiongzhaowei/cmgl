//
//  Renderer.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/14.
//

OMP_RENDER_NAMESPACE_BEGIN

struct RenderContext;
struct Framebuffer;

struct Renderer : public Resource {

    typedef enum {
        None,
        RGBA,
        YUV420P,
        YUV420SP,
    } Name;

    static const mat4 kColorConversionNone;
    static const mat4 kColorConversionExchangeRedAndBlue;
    static const mat4 kColorConversionBT601;
    static const mat4 kColorConversionBT601FullRange;
    static const mat4 kColorConversionBT709;

    virtual void draw(
        RefPtr<RenderContext> context,
        RefPtr<Framebuffer> framebuffer,
        const mat4 &globalMatrix,
        const mat4 &localMatrix,
        const mat4 &clipMatrix,
        const mat4 &colorConversion,
        const vec2 &size,
        float alpha,
        uint32_t texture,
        va_list list
    ) = 0;

    static RefPtr<Renderer> createRenderer(Name);
};

OMP_RENDER_NAMESPACE_END
