//
// Created by 熊朝伟 on 2020-04-30.
//

#pragma once

OMP_RENDER_MS_NAMESPACE_BEGIN

class OSRenderContext : public egl::EGLRenderContext {
public:
	OSRenderContext(EGLNativeDisplayType nativeDisplay = EGL_DEFAULT_DISPLAY);
};

OMP_RENDER_MS_NAMESPACE_END
