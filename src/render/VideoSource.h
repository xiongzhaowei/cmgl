//
// Created by 熊朝伟 on 2020/3/25.
//

#pragma once

struct AVFrame;

OMP_RENDER_NAMESPACE_BEGIN

// struct VideoSource;

// struct VideoFormat {
//     virtual void update(RefPtr<VideoSource> source, const AVFrame *frame) = 0;
//     virtual void draw(
//         RefPtr<RenderContext> context,
//         RefPtr<Framebuffer> framebuffer,
//         const mat4 &globalMatrix,
//         const mat4 &localMatrix,
//         const mat4 &clipMatrix,
//         const vec2 &size,
//         float alpha
//     ) = 0;

//     static std::unique_ptr<VideoFormat> RGBA;
//     static std::unique_ptr<VideoFormat> RGB24;
//     static std::unique_ptr<VideoFormat> YU12;
//     static std::unique_ptr<VideoFormat> YV12;
//     static std::unique_ptr<VideoFormat> NV12;
//     static std::unique_ptr<VideoFormat> NV21;
// };

struct VideoSource : public DataSource {

    // struct Planar {
    //     RefPtr<Texture> texture;
    //     std::vector<uint8_t> pixels;
    // };

    typedef enum {
        Unknown,
        RGBA,
        RGB24,
        YUV420P,
        YUV420SP,
    } Type;

    VideoSource();
    vec2 size() const override;

    virtual void update(const AVFrame *frame) = 0;
    virtual bool support(const Type &type) = 0;

#ifdef __ANDROID__
    static VideoSource *fromObject(JNIEnv *env, jobject obj);
    static void setHandle(JNIEnv *env, jobject obj, VideoSource *source);
#endif
protected:
    ivec2 _size = ivec2(0, 0);
    std::atomic<bool> _isNeedsUpdate;
    // std::vector<Planar> _planars;
};

OMP_RENDER_NAMESPACE_END
