//
//  OMPRenderContext.mm
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#import "defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

@implementation OMPRenderContext {
    RefPtr<RenderContext> _context;
}

- (instancetype)init {
    if (self = [super init]) {
        _context = new AppleRenderContext;
    }
    return self;
}

- (instancetype)initWithHandle:(void *)handle {
    if (self = [super init]) {
        _context = (RenderContext *)handle;
    }
    return self;
}

- (void)load {
    _context->load();
}

- (void)unload {
    _context->unload();
}

- (void)render {
    _context->render();
}

- (void)makeCurrent {
    _context->makeCurrent();
}

- (void)addNativeSource:(void *)source {
    _context->addSource((RenderSource *)source);
}

- (void)setNativeWindow:(nullable void *)window {
    _context->setTarget((RenderTarget *)window);
}

@end
