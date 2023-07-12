//
//  YUV420PVideoSource.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct YUV420PVideoSource : public VideoSource {
    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
    void clear(uint8_t red, uint8_t green, uint8_t blue);
    void update(const AVFrame *frame) override;
    bool support(const Type &type) override;
    void draw(
        RefPtr<RenderContext> context,
        RefPtr<Framebuffer> framebuffer,
        const mat4 &globalMatrix,
        const mat4 &localMatrix,
        const mat4 &clipMatrix,
        const vec2 &size,
        float alpha
    ) override;

protected:
    RefPtr<Texture> _texture1;
    RefPtr<Texture> _texture2;
    RefPtr<Texture> _texture3;
    std::vector<uint8_t> _pixels1;
    std::vector<uint8_t> _pixels2;
    std::vector<uint8_t> _pixels3;
    std::mutex _lock;
};

OMP_RENDER_NAMESPACE_END
