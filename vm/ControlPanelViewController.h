#import <AppKit/NSViewController.h>

struct Machine;

@interface ControlPanelViewController : NSViewController

- (instancetype)initWithMachine:(Machine *const)machine;

@end
