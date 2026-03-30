#import "ControlPanelViewController.h"

#import "ControlPanelView.h"
#import "VirtualMachine.hpp"

@interface ControlPanelViewController () {
    VirtualMachine *_machine;
}
@end

@interface ControlPanelViewController (ControlPanelDelegate) <ControlPanelDelegate>
@end

@implementation ControlPanelViewController (ControlPanelDelegate)

- (void)controlPanelDidRequestAction:(enum ControlPanelAction)action {
    self->_machine->run();
}

@end

@implementation ControlPanelViewController

- (instancetype)initWithMachine:(VirtualMachine *const)machine {
    self = [super init];
    if (self != nil) {
        self->_machine = machine;
    }
    return self;
}

- (ControlPanelView *)controlPanel {
    return (ControlPanelView *)self.view;
}

- (void)loadView {
    self.view = [[ControlPanelView alloc] init];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    ControlPanelView *const controlPanelView = [self controlPanel];
    controlPanelView->delegate = self;
}

@end
