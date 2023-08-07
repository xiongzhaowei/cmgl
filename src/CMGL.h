//
//  CMGL.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#pragma once

#include "defines.h"
#include "render/defines.h"
#include "render/gles2/defines.h"
#include "render/microsoft/defines.h"
#include "ffmpeg/defines.h"
#include "ui/defines.h"
#include "ui/windows/defines.h"
#include "ui/windows/render/defines.h"

#ifdef __cplusplus

namespace cmgl {
    using wheel::RefPtr;
    using wheel::WeakPtr;
    using wheel::Object;

    using wheel::render::RenderContext;
    using wheel::render::RenderTarget;
    using wheel::render::RenderFactory;
    using wheel::render::RenderObject;
    using wheel::render::gles2::TestRenderObject;

    namespace win32 {
        using namespace wheel::render::microsoft;
    }

    typedef RefPtr<RenderContext> RenderContextPtr;
    typedef RefPtr<RenderTarget> RenderTargetPtr;
    typedef RefPtr<RenderObject> RenderObjectPtr;
}

#endif
