//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct RenderSource : public Resource {
    virtual bool enabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual vec2 size() const = 0;
    virtual void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) = 0;
};

struct RenderTarget : public Resource {
    virtual vec2 size() const = 0;
    virtual void startRender(RefPtr<RenderContext> context) = 0;
    virtual void render(RefPtr<RenderContext> context, RefPtr<RenderSource> source) = 0;
    virtual void finishRender(RefPtr<RenderContext> context) = 0;
};

struct RenderFilter : public RenderSource, public RenderTarget {
    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
    vec2 size() const override;
    void startRender(RefPtr<RenderContext> context) override;
    void finishRender(RefPtr<RenderContext> context) override;
    void render(RefPtr<RenderContext> context, RefPtr<RenderSource> source) override;
    void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) override;
protected:
    ivec2 _size = ivec2(0, 0);
    RefPtr<Framebuffer> _framebuffer;
};

struct DataSource : public RenderSource {
    bool enabled() const override;
    void setEnabled(bool enabled) override;
    void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) override;

    virtual void draw(
        RefPtr<RenderContext> context,
        RefPtr<Framebuffer> framebuffer,
        const mat4 &globalMatrix,
        const mat4 &localMatrix,
        const mat4 &clipMatrix,
        const vec2 &size,
        float alpha
    ) = 0;

protected:
    bool _enabled = true;
};

OMP_RENDER_NAMESPACE_END
