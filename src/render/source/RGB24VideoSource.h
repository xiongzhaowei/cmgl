﻿//
//  RGB24VideoSource.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/22.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct RGB24VideoSource : public VideoSource {
    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
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

#if defined(_WIN32) && defined(_GDIPLUS_H)
    void update(Gdiplus::Bitmap* bitmap);
#endif
protected:
    RefPtr<Texture> _texture;
    std::vector<uint8_t> _pixels;
    std::mutex _lock;
};

OMP_RENDER_NAMESPACE_END
