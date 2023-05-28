//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct RenderContext;

struct Framebuffer : public Object {
    virtual int32_t width() const = 0;
    virtual int32_t height() const = 0;

    virtual void bind(RefPtr<RenderContext> context) = 0;
    virtual void unbind(RefPtr<RenderContext> context) = 0;

    virtual uint32_t texture() { return 0; }
};

OMP_RENDER_NAMESPACE_END
