//
//  AppleRenderContext.h
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

OMP_RENDER_APPLE_NAMESPACE_BEGIN

class AppleFramebuffer;

class AppleRenderContext : public RenderContext {
    RefPtr<EAGLContext> _context;
public:

    void load();
    void unload();
    void render() override;
    void present() override;
    
    RefPtr<Framebuffer> createFramebuffer(const void *window) override;
    void setFramebuffer(RefPtr<Framebuffer> framebuffer) override;

    void makeCurrent() override;

};

OMP_RENDER_APPLE_NAMESPACE_END

OMPLAYER_NAMESPACE_BEGIN

template <> void RefPtr<EAGLContext>::retain(EAGLContext *value);
template <> bool RefPtr<EAGLContext>::release(EAGLContext *value);
template <> void RefPtr<EAGLContext>::destroy(EAGLContext *value);

OMPLAYER_NAMESPACE_END
