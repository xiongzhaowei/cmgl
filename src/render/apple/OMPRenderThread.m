//
//  OMPRenderThread.m
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#import "defines.h"
#import <CoreVideo/CoreVideo.h>

static NSMutableDictionary<NSString *, OMPRenderThread *> *_threads;

@implementation OMPRenderThread {
    NSThread *_thread;
    NSTimer *_timer;
    OMPRenderContext *_renderContext;
}

+ (instancetype)thread:(NSString *)name {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _threads = [NSMutableDictionary dictionary];
    });
    OMPRenderThread *thread = [_threads objectForKey:name];
    if (!thread) {
        thread = [[OMPRenderThread alloc] initWithName:name];
        [_threads setObject:thread forKey:name];
    }
    return thread;
}

- (instancetype)initWithName:(NSString *)name {
    assert([_threads objectForKey:name] == nil);
    if (self = [super init]) {
        _thread = [[NSThread alloc] initWithTarget:self selector:@selector(renderThread) object:nil];
    }
    return self;
}

- (void)start {
    [_thread start];
}

- (void)stop {
    [self runOnRenderThread:^(OMPRenderContext *context) {
        [_timer invalidate];
        CFRunLoopStop(CFRunLoopGetCurrent());
    }];
}

- (void)runOnRenderThread:(void(^)(OMPRenderContext *context))callback {
    if ([[NSThread currentThread] isEqual:_thread]) {
        [self render:callback];
    } else {
        [self performSelector:@selector(render:)
                     onThread:_thread
                   withObject:callback
                waitUntilDone:NO];
    }
}

- (void)render:(void(^)(OMPRenderContext *context))callback {
    [_renderContext makeCurrent];
    callback(_renderContext);
}

- (void)render {
    [self runOnRenderThread:^(OMPRenderContext *context) { [context render]; }];
}

static void DoNothingRunLoopCallback(void *info) {
}

- (void)renderThread {
    @autoreleasepool {
        CFRunLoopSourceContext context = {0};
        context.perform = DoNothingRunLoopCallback;
        CFRunLoopSourceRef source = CFRunLoopSourceCreate(NULL, 0, &context);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
        
        _renderContext = [[OMPRenderContext alloc] init];
        [_renderContext load];
        _timer = [NSTimer timerWithTimeInterval:0.016666666 target:self selector:@selector(render) userInfo:nil repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:_timer forMode:NSRunLoopCommonModes];
        
        CFRunLoopRun();
        
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
        CFRelease(source);
    }
}

@end
