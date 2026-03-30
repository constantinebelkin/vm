#import <AppKit/NSViewController.h>

struct VirtualDisplay;

@interface DisplayViewController : NSViewController

NS_ASSUME_NONNULL_BEGIN

- (instancetype)initWithDisplay:(struct VirtualDisplay *const)display;

NS_ASSUME_NONNULL_END

@end
