#import "RootView.h"

@implementation RootView {
    NSView *_controlPanelView;
    NSView *_displayView;
}

- (void)setControlPanelView:(NSView *const)controlPanelView {
    self->_controlPanelView = controlPanelView;
    [self addSubview:controlPanelView];
    [self needsLayout];
}

- (void)setDisplayView:(NSView *const)displayView {
    self->_displayView = displayView;
    [self addSubview:displayView];
    [self needsLayout];
}

- (void)layout {
    [super layout];

    const NSRect rootViewBounds = self.bounds;

    const NSRect displayViewFrame = {
        .origin = {
            .x = rootViewBounds.origin.x,
            .y = rootViewBounds.origin.y
        },
        .size = {
            .width = rootViewBounds.size.width,
            .height = 800.0
        }
    };
    const NSRect controlPanelViewFrame = {
        .origin = {
            .x = rootViewBounds.origin.x,
            .y = displayViewFrame.size.height
        },
        .size = {
            .width = rootViewBounds.size.width,
            .height = rootViewBounds.size.height - displayViewFrame.size.height
        }
    };

    self->_controlPanelView.frame = controlPanelViewFrame;
    self->_displayView.frame = displayViewFrame;
}

@end
