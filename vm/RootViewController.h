#import <AppKit/NSViewController.h>

struct Machine;

@interface RootViewController : NSViewController

NS_ASSUME_NONNULL_BEGIN

- (instancetype)initWithMachine:(struct Machine *const)machine;

NS_ASSUME_NONNULL_END

@end
