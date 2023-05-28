//
//  OMPRenderView.h
//  omgl
//
//  Created by 熊朝伟 on 2020/3/30.
//

NS_ASSUME_NONNULL_BEGIN

@interface OMPRenderView : UIView

+ (NSString *)threadName;

- (void)runOnRenderThread:(void(^)(OMPRenderContext *context))renderCallback;

@end

NS_ASSUME_NONNULL_END
