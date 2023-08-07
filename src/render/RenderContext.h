//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct RenderContext : public Object {
    virtual RefPtr<Framebuffer> framebuffer();
    virtual void setFramebuffer(RefPtr<Framebuffer> framebuffer);

    virtual void load();
    virtual void unload();

    virtual RefPtr<RenderSource> source(int32_t index);
    virtual void addSource(RefPtr<RenderSource> source);
    virtual void removeSource(RefPtr<RenderSource> source);
    virtual size_t numberOfSources() const;

    virtual RefPtr<RenderTarget> target();
    virtual void setTarget(RefPtr<RenderTarget> target);

    virtual void notifySourceChanged();
    virtual void onSourceChanged(std::function<void(RenderContext* context)> callback);

    void draw(
        Renderer::Name name,
        RefPtr<Framebuffer> framebuffer,
        const mat4 &globalMatrix,
        const mat4 &localMatrix,
        const mat4 &clipMatrix,
        const mat4 &colorConversion,
        const vec2 &size,
        float alpha,
        ...
    );

    virtual void makeCurrent() = 0;
    virtual void render() = 0;
    virtual void present() = 0;
    virtual RefPtr<Framebuffer> createFramebuffer(const void *window) = 0;
    virtual RefPtr<Framebuffer> createFramebuffer(int32_t width, int32_t height);
    virtual RefPtr<Texture> createTexture();

    static RefPtr<RenderTarget> createWindow(const void *window);
protected:
    std::function<void(RenderContext* context)> _sourceChangedCallback;
    std::vector<RefPtr<RenderSource>> _renderSources;
    std::unordered_map<Renderer::Name, RefPtr<Renderer>> _renderers;
    RefPtr<RenderTarget> _renderTarget;
    WeakPtr<Framebuffer> _framebuffer;
    bool _isEnabled = false;
};

OMP_RENDER_NAMESPACE_END
