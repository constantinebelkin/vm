#import "RootViewController.h"

#import "ControlPanelViewController.h"
#import "DisplayViewController.h"
#import "VirtualDisplay.hpp"
#import "VirtualKeyboard.hpp"
#import "VirtualMachine.hpp"
#import "RootView.h"

@interface RootViewController () {
@private
    struct VirtualMachine *_machine;
}

@property (readonly) RootView *rootView;

@end

@implementation RootViewController

- (instancetype)initWithMachine:(struct VirtualMachine *const)machine {
    self = [super init];
    if (self) {
        self->_machine = machine;
    }
    return self;
}

- (RootView *)rootView {
    return (RootView *)self.view;
}

- (void)loadView {
    self.view = [[RootView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 1280.0, 860.0)];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    auto *const controlPanelViewController = [[ControlPanelViewController alloc] initWithMachine:self->_machine];
    [self addChildViewController:controlPanelViewController];
    [[self rootView] setControlPanelView:controlPanelViewController.view];

    auto *const displayViewController = [[DisplayViewController alloc] initWithDisplay:self->_machine->display.get()];
    [self addChildViewController:displayViewController];
    [self.rootView setDisplayView:displayViewController.view];
}

- (void)keyDown:(NSEvent *)event {
    self->_machine->keyboard.get()->keyDown(event.keyCode);
}

@end
