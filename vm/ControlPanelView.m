#import "ControlPanelView.h"

#import <AppKit/NSColor.h>
#import <AppKit/NSButton.h>
#import <QuartzCore/CALayer.h>

@implementation ControlPanelView {
@private
    NSButton *_machineRunButton;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        [self setWantsLayer:YES];
        self->_machineRunButton = [NSButton buttonWithTitle:@"Run machine"
                                                     target:self
                                                     action:@selector(onMachineRunButtonClicked:)];
        [self addArrangedSubview:self->_machineRunButton];
    }
    return self;
}

- (void)onMachineRunButtonClicked:(NSButton *const)sender {
    [self->delegate controlPanelDidRequestAction:ControlPanelActionMachineRun];
}

@end
