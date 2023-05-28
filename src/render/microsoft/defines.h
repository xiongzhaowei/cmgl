//
// Created by 熊朝伟 on 2020-03-19.
//

#pragma once

#include "../egl/defines.h"

#define OMP_RENDER_MS_NAMESPACE_BEGIN    OMP_RENDER_NAMESPACE_BEGIN namespace microsoft {
#define OMP_RENDER_MS_NAMESPACE_END      } OMP_RENDER_NAMESPACE_END
#define OMP_RENDER_MS_USING_NAMESPACE    OMP_RENDER_USING_NAMESPACE using namespace OMP_RENDER_NAMESPACE_PREFIX microsoft;

#include "RenderContext.h"
#include "Window.h"
