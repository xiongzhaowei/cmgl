//
//  defines.h
//  omplayer
//
//  Created by 熊朝伟 on 2023/5/11.
//

#pragma once

#include "../defines.h"
#include <optional>

#define OMP_UI_NAMESPACE_BEGIN    OMP_NAMESPACE_BEGIN namespace ui {
#define OMP_UI_NAMESPACE_END      } OMP_NAMESPACE_END
#define OMP_UI_NAMESPACE_PREFIX   OMP_NAMESPACE_PREFIX ui ::
#define OMP_UI_USING_NAMESPACE    OMP_USING_NAMESPACE using namespace OMP_NAMESPACE_PREFIX ui;

#include "../render/egl/defines.h"
#include "Type.h"
#include "Layer.h"
#include "View.h"
#include "LayoutConstraint.h"
