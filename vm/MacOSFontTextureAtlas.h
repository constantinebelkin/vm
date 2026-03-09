#import <Foundation/Foundation.h>

@protocol MTLDevice;
@protocol MTLTexture;

@interface MacOSFontTextureAtlas : NSObject

NS_ASSUME_NONNULL_BEGIN

- (instancetype)initWithDevice:(id<MTLDevice>)device;
- (id<MTLTexture>)getTexture;

NS_ASSUME_NONNULL_END

@end
