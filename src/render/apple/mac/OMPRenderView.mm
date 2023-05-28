//
//  OMPRenderView.mm
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/1.
//

#import "../defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

@interface OMPRenderView () {
@public
    RefPtr<AppleWindow> _window;
}

@end

static RefPtr<AppleWindow> createNativeWindow(OMPRenderView *self) {
    OMPRenderThread *thread = [OMPRenderThread thread:@"OMPRenderThread"];
    self.wantsLayer = YES;
    RefPtr<AppleWindow> window = new AppleWindow(self.layer);
    [thread runOnRenderThread:^(OMPRenderContext *context) {
        [context setNativeWindow:window];
    }];
    [thread start];
    return window;
}

static void setTestVideoSource(RefPtr<VideoSource> source) {
    uint32_t pixels[64 * 64];
    for (uint32_t &pixel : pixels) {
        pixel = 0xFFFFFF00;
    }
    source->update(64, 64, pixels);
}

static void initTestCode(OMPRenderView *self) {
    RefPtr<URLVideoSource> source = new URLVideoSource;
    //在macOS端网络访问有问题，只能读取本地文件。并且因为沙箱的缘故，文件必须放在Data目录下，否则无法读取。
    std::string url = "/Users/xiongzhaowei/Library/Containers/com.cmcm.OMPRenderDemo/Data/Documents/test.mp4";
    //std::string url = "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/prog_index.m3u8";
    //source->open(url, VideoSource::RGB24);
    source->open(url, VideoSource::YUV420P);
    //source->open(url, VideoSource::YUV420SP);
    //setTestVideoSource(source);
    
    RefPtr<RenderLayer> layer = new RenderLayer;
//    layer->setAnchorPoint(vec2(0.5, 0.5));
    layer->setPosition(vec2(250, 250));
    layer->setSize(vec2(150, 150));
    layer->setRotate(vec3(0, 0, 3.1415926 / 4));
    layer->setOffset(vec2(40, 50));
    layer->setSource(source);
    layer->applyTransformScale(vec2(2, 2));
//    layer->setMaskToBounds(false);
    
    OMPRenderThread *thread = [OMPRenderThread thread:@"OMPRenderThread"];
    [thread runOnRenderThread:^(OMPRenderContext *context) {
        [context addNativeSource:source];
    }];
    
    // 播放20秒后自动停止播放，测试停止播放代码
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(20 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        source->close();
    });
}

@implementation OMPRenderView {
    __weak OMPRenderThread *_thread;
}

- (instancetype)init {
    if (self = [super init]) {
        initTestCode(self);
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        initTestCode(self);
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    if (self = [super initWithCoder:coder]) {
        initTestCode(self);
    }
    return self;
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow {
    [super viewWillMoveToWindow:newWindow];

    _window = createNativeWindow(self);
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (CALayer *)makeBackingLayer {
    return [[CAOpenGLLayer alloc] init];
}

@end
