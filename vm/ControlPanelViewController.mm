#import "ControlPanelViewController.h"

#import "ControlPanelView.h"
#import "Machine.hpp"

@interface ControlPanelViewController () {
    Machine *_machine;
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

- (instancetype)initWithMachine:(Machine *const)machine {
    self = [super init];
    if (self != nil) {
        self->_machine = machine;
    }
    return self;
}

- (ControlPanelView *)controlPanel {
    return (ControlPanelView *)[self view];
}

- (void)loadView {
    [self setView:[[ControlPanelView alloc] init]];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    ControlPanelView *const controlPanelView = [self controlPanel];
    controlPanelView->delegate = self;
}

@end
