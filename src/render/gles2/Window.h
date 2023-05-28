//
//  Window.hpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#pragma once

OMP_RENDER_GLES2_NAMESPACE_BEGIN

class Window : public RenderTarget {
protected:
    RefPtr<Framebuffer> _framebuffer;
    mat4 _projectionMatrix;
public:
    void startRender(RefPtr<RenderContext> context) override;
    void render(RefPtr<RenderContext> context, RefPtr<RenderSource> source) override;
    void finishRender(RefPtr<RenderContext> context) override;
};

OMP_RENDER_GLES2_NAMESPACE_END
