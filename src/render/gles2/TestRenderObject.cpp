//
//  RenderObject.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#include "defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

void TestRenderObject::load(RefPtr<RenderContext> context) {

}

void TestRenderObject::unload(RefPtr<RenderContext> context) {

}

void TestRenderObject::render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) {
    GL_ERROR(glClearColor(0.5f, 0.5f, 1.0f, 1.0f));
    GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

std::string TestRenderObject::name() {
    return "TestRenderObject";
}

RefPtr<RenderObject> TestRenderObject::make(const std::string &json) {
    return new TestRenderObject;
}

OMPDefineRenderObject(TestRenderObject)
