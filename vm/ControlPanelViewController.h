#import <AppKit/NSViewController.h>

struct VirtualMachine;

@interface ControlPanelViewController : NSViewController

- (instancetype)initWithMachine:(VirtualMachine *const)machine;

@end
