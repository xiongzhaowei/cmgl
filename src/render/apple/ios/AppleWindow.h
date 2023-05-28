//
//  AppleWindow.h
//  IJKMediaFramework
//
//  Created by 熊朝伟 on 2020/3/27.
//  Copyright © 2020 OMP. All rights reserved.
//

#pragma once

OMP_RENDER_APPLE_NAMESPACE_BEGIN

class AppleWindow : public gles2::Window {
    RefPtr<CALayer> _window;
public:
    AppleWindow(CALayer *window);

    vec2 size() const override;

    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
};

OMP_RENDER_APPLE_NAMESPACE_END

OMPLAYER_NAMESPACE_BEGIN

template <> void RefPtr<CALayer>::retain(CALayer *value);
template <> bool RefPtr<CALayer>::release(CALayer *value);
template <> void RefPtr<CALayer>::destroy(CALayer *value);

OMPLAYER_NAMESPACE_END
