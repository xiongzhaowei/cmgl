//
//  URLVideoSource.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/16.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

// 目前仅用于渲染层代码自测，只保证图像能正确播放，不处理音频播放和帧同步问题。
struct URLVideoSource : public VideoSource {
    class Decoder;
    class VideoConverter;

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

    bool open(const std::string &url, const Type &type = YUV420P);
    void close();
protected:
    WeakPtr<RenderContext> _context;
    std::atomic<bool> _isDecoding;
    std::shared_ptr<Decoder> _decoder;
    RefPtr<VideoSource> _source;
};

OMP_RENDER_NAMESPACE_END
