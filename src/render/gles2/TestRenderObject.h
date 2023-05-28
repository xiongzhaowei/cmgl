//
//  RenderObject.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#pragma once

OMP_RENDER_GLES2_NAMESPACE_BEGIN

class TestRenderObject : public RenderObject {
public:
    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
    void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) override;
    
    static std::string name();
    static RefPtr<RenderObject> make(const std::string &json);
};

OMP_RENDER_GLES2_NAMESPACE_END
