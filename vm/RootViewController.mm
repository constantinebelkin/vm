#import "RootViewController.h"

#import "ControlPanelViewController.h"
#import "DisplayViewController.h"
#import "Display.hpp"
#import "Keyboard.hpp"
#import "Machine.hpp"
#import "RootView.h"

@interface RootViewController () {
@private
    struct Machine *_machine;
}

@end

@interface RootViewController (DisplayInputDelegate) <DisplayInputDelegate>
@end

@implementation RootViewController (DisplayInputDelegate)

- (void)displayDidCatchInputWithEvent:(NSEvent *const)event {
    NSData *const data = [[event characters] dataUsingEncoding:NSWindowsCP1251StringEncoding allowLossyConversion:YES];
    self->_machine->display->print_characters((uint8_t *)[data bytes], [data length]);
}

@end

@implementation RootViewController

- (instancetype)initWithMachine:(struct Machine *const)machine {
    self = [super init];
    if (self) {
        self->_machine = machine;
    }
    return self;
}

- (RootView *)rootView {
    return (RootView *)[self view];
}

- (void)loadView {
    [self setView:[[RootView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 1280.0, 860.0)]];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    auto *const controlPanelViewController = [[ControlPanelViewController alloc] initWithMachine:self->_machine];
    [self addChildViewController:controlPanelViewController];
    [[self rootView] setControlPanelView:[controlPanelViewController view]];

    auto *const displayViewController = [[DisplayViewController alloc] initWithDisplay:self->_machine->display];
    [self addChildViewController:displayViewController];
    [[self rootView] setDisplayView:[displayViewController view]];
    displayViewController->displayInputDelegate = self;
}

@end
