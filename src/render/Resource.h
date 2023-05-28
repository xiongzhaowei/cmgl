//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

struct RenderContext;

struct Resource : public Object {
    virtual void load(RefPtr<RenderContext> context) = 0;
    virtual void unload(RefPtr<RenderContext> context) = 0;
};

OMP_RENDER_NAMESPACE_END
