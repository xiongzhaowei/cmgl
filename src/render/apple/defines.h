//
//  defines.h
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

#include "../defines.h"
#include "../gles2/defines.h"

#ifdef __OBJC__
    #import <Foundation/Foundation.h>
    #import <QuartzCore/QuartzCore.h>

    #if TARGET_OS_IOS
        #import <UIKit/UIKit.h>
        #import <OpenGLES/EAGL.h>
    #elif TARGET_OS_OSX
        #import <Cocoa/Cocoa.h>
    #endif
#endif

#define OMP_RENDER_APPLE_NAMESPACE_BEGIN    OMP_RENDER_NAMESPACE_BEGIN namespace apple {
#define OMP_RENDER_APPLE_NAMESPACE_END      } OMP_RENDER_NAMESPACE_END
#define OMP_RENDER_APPLE_USING_NAMESPACE    OMP_RENDER_USING_NAMESPACE using namespace OMP_RENDER_NAMESPACE_PREFIX apple;

#ifdef __cplusplus
    #if TARGET_OS_IOS
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
        #include "ios/AppleRenderContext.h"
        #include "ios/AppleFramebuffer.h"
        #include "ios/AppleWindow.h"
    #elif TARGET_OS_OSX
        #include "../egl/defines.h"
        #include "mac/AppleWindow.h"
        OMP_RENDER_APPLE_NAMESPACE_BEGIN
            using AppleRenderContext = egl::EGLRenderContext;
        OMP_RENDER_APPLE_NAMESPACE_END
    #endif
#endif

#ifdef __OBJC__
    #import "OMPRenderContext.h"
    #import "OMPRenderThread.h"
    #if TARGET_OS_IOS
        #import "ios/OMPRenderView.h"
        #import "ios/OMPView.h"
    #elif TARGET_OS_OSX
        #import "mac/OMPRenderView.h"
    #endif
#endif
