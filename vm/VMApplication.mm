#import "VMApplication.h"

#import <AppKit/NSWindow.h>

#import "VirtualMachine.hpp"
#import "RootViewController.h"

@implementation VMApplication {
    std::unique_ptr<struct VirtualMachine> _machine;
    NSWindow *_mainWindow;
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    NSString *const firmwarePath = [NSBundle.mainBundle pathForResource:@"firmware" ofType:@"bin"];
    if (firmwarePath == nil) {
        NSLog(@"Firmware not found");
    }

    self->_machine = VirtualMachine::create([firmwarePath cStringUsingEncoding:NSASCIIStringEncoding]);
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    self->_mainWindow = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                    styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];

    self->_mainWindow.contentViewController = [[RootViewController alloc] initWithMachine:self->_machine.get()];
    self->_mainWindow.title = @"Virtual Machine";
    [self->_mainWindow makeKeyAndOrderFront:self];
}

@end
