//
//  Window.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#include "defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

void Window::render(RefPtr<RenderContext> context, RefPtr<RenderSource> source) {
    if (source) source->render(context, _framebuffer, _projectionMatrix);
}

void Window::startRender(RefPtr<RenderContext> context) {
    context->makeCurrent();
    context->setFramebuffer(_framebuffer);
    vec2 size = this->size();
    _projectionMatrix = glm::ortho(0.0f, size.x, size.y, 0.0f);
    GL_ERROR(glViewport(0, 0, size.x, size.y));
    GL_ERROR(glClearColor(0.3f, 0.3f, 0.3f, 1));
    GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    // 开启预乘模式alpha混合，需要在片元着色器中输出预乘后的颜色
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
}

void Window::finishRender(RefPtr<RenderContext> context) {
    context->present();
}
