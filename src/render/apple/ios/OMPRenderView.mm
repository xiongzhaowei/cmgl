//
//  OMPlayerView.m
//  omgl
//
//  Created by 熊朝伟 on 2020/3/30.
//

#import "../defines.h"
#import "OMPRenderView.h"

OMP_RENDER_APPLE_USING_NAMESPACE

@implementation OMPRenderView {
    RefPtr<AppleWindow> _window;
}

static void initView(OMPRenderView *self) {
    self.contentScaleFactor = UIScreen.mainScreen.scale;
    
    OMPRenderThread *thread = [OMPRenderThread thread:[self.class threadName]];
    RefPtr<AppleWindow> window = new AppleWindow(self.layer);
    [thread runOnRenderThread:^(OMPRenderContext *context) {
        [context setNativeWindow:window];
    }];
    [thread start];
    
    self->_window = window;
}

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

+ (NSString *)threadName {
    return @"OMPRenderThread";
}

- (instancetype)init {
    if (self = [super init]) {
        initView(self);
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        initView(self);
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    if (self = [super initWithCoder:coder]) {
        initView(self);
    }
    return self;
}

- (void)dealloc {
    [self runOnRenderThread:^(OMPRenderContext *context) {
        [context setNativeWindow:nil];
    }];
}

- (void)runOnRenderThread:(void(^)(OMPRenderContext *context))renderCallback {
    OMPRenderThread *thread = [OMPRenderThread thread:[self.class threadName]];
    [thread runOnRenderThread:renderCallback];
}

@end
