//
//  defines.h
//  omplayer
//
//  Created by 熊朝伟 on 2023/5/11.
//

#include "../defines.h"
#include <Windows.h>
#include <atlcoll.h>
#include <atlcomcli.h>
#include <gdiplus.h>

#define OMP_UI_WINDOWS_NAMESPACE_BEGIN      OMP_UI_NAMESPACE_BEGIN namespace windows {
#define OMP_UI_WINDOWS_NAMESPACE_END        } OMP_UI_NAMESPACE_END
#define OMP_UI_WINDOWS_NAMESPACE_PREFIX     OMP_UI_NAMESPACE_PREFIX windows ::
#define OMP_UI_WINDOWS_USING_NAMESPACE      OMP_UI_USING_NAMESPACE using namespace OMP_UI_NAMESPACE_PREFIX windows;

//#include "Pointer.h"
#include "Layer.h"
#include "Window.h"
