#import <AppKit/NSViewController.h>

struct VirtualMachine;

@interface RootViewController : NSViewController

NS_ASSUME_NONNULL_BEGIN

- (instancetype)initWithMachine:(struct VirtualMachine *const)machine;

NS_ASSUME_NONNULL_END

@end
