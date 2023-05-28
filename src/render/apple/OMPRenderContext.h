//
//  OMPRenderContext.h
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

NS_ASSUME_NONNULL_BEGIN

@interface OMPRenderContext : NSObject

- (instancetype)init;
- (instancetype)initWithHandle:(void *)handle;

- (void)load;
- (void)unload;
- (void)render;
- (void)makeCurrent;

- (void)addNativeSource:(void *)source;
- (void)setNativeWindow:(nullable void *)window;

@end

NS_ASSUME_NONNULL_END
