//
//  OMPView.m
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#import "../defines.h"

OMP_RENDER_APPLE_USING_NAMESPACE

@implementation OMPView {
    RefPtr<RenderObject> _renderObject;
}

static void initView(OMPRenderView *self) {
    [self addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(onTap:)]];
    [self addGestureRecognizer:[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onPan:)]];
    [self addGestureRecognizer:[[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)]];
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

- (void)setName:(NSString *)name {
    _name = name;
    _renderObject = RenderFactory::make(name.UTF8String, "");
    [self setNeedsLayout];
    [self layoutIfNeeded];
}

- (BOOL)hitTest:(CGPoint)point {
    if (!_renderObject) {
        return NO;
    }

    CGFloat scale = self.contentScaleFactor;
    point.x *= scale;
    point.y *= scale;

    return _renderObject->hitTest(point.x, point.y);
}

- (void)onPan:(UIPanGestureRecognizer *)sender {
    RefPtr<RenderObject> renderObject = _renderObject;
    if (!renderObject) return;
    
    CGFloat scale = self.contentScaleFactor;
    CGPoint point = [sender locationInView:self];
    point.x *= scale;
    point.y *= scale;

    switch (sender.state) {
        case UIGestureRecognizerStateBegan: {
            [self runOnRenderThread:^(OMPRenderContext *context) {
                PanGestureBeganEvent event;
                event.x = point.x;
                event.y = point.y;
                renderObject->events().publish(event);
            }];
            break;
        }
        case UIGestureRecognizerStateChanged: {
            [self runOnRenderThread:^(OMPRenderContext *context) {
                PanGestureMovedEvent event;
                event.x = point.x;
                event.y = point.y;
                renderObject->events().publish(event);
            }];
            break;
        }
        case UIGestureRecognizerStateEnded: {
            [self runOnRenderThread:^(OMPRenderContext *context) {
                PanGestureEndedEvent event;
                event.x = point.x;
                event.y = point.y;
                renderObject->events().publish(event);
            }];
            break;
        }
        default:
            break;
    }
}

@end
