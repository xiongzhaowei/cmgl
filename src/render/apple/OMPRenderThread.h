//
//  OMPRenderThread.h
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

NS_ASSUME_NONNULL_BEGIN

@interface OMPRenderThread : NSObject

+ (instancetype)thread:(NSString *)name;

- (void)start;
- (void)stop;

- (void)runOnRenderThread:(void(^)(OMPRenderContext *context))callback;

@end

NS_ASSUME_NONNULL_END
