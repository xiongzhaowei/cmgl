//
//  OMPView.h
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

NS_ASSUME_NONNULL_BEGIN

@interface OMPView : OMPRenderView

@property (nonatomic, readonly) OMPRenderThread *thread;
@property (nonatomic, nullable) IBInspectable NSString *name; // 通过name设置renderObject属性
#ifdef __cplusplus
//@property (nonatomic, readonly) omplayer::RefPtr<omplayer::render::::RenderObject> renderObject;
#endif

@end

NS_ASSUME_NONNULL_END
