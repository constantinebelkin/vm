#import "VMApplication.h"

#import <AppKit/NSWindow.h>

#import "Machine.hpp"
#import "RootViewController.h"

@implementation VMApplication {
    struct Machine *_machine;
    NSWindow *_mainWindow;
}

- (void)dealloc {
    if (self->_mainWindow != nil) {
        [self->_mainWindow dealloc];
    }

    if (self->_machine != nullptr) {
        delete self->_machine;
    }

    [super dealloc];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    NSString *const firmwarePath = [[NSBundle mainBundle] pathForResource:@"firmware" ofType:@"bin"];
    if (firmwarePath == nil) {
        NSLog(@"Firmware not found");
    }

    self->_machine = new Machine([firmwarePath cStringUsingEncoding:NSASCIIStringEncoding]);
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self->_mainWindow = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                    styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];

    [self->_mainWindow setContentViewController:[[RootViewController alloc] initWithMachine:self->_machine]];
    [self->_mainWindow setTitle:@"Virtual Machine"];
    [self->_mainWindow makeKeyAndOrderFront:self];
}

@end
