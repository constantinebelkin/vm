#import <AppKit/NSStackView.h>

enum ControlPanelAction {
    ControlPanelActionMachineRun
};

@protocol ControlPanelDelegate

- (void)controlPanelDidRequestAction:(enum ControlPanelAction)action;

@end

@interface ControlPanelView : NSStackView {
@public
    __weak id<ControlPanelDelegate> delegate;
}

@end
