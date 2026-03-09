#import <AppKit/NSViewController.h>

struct Display;

NS_ASSUME_NONNULL_BEGIN

@protocol DisplayInputDelegate

- (void)displayDidCatchInputWithEvent:(NSEvent *const)event;

@end

@interface DisplayViewController : NSViewController {
@public
    __weak id<DisplayInputDelegate> displayInputDelegate;
}

- (instancetype)initWithDisplay:(struct Display *const)display;

@end

NS_ASSUME_NONNULL_END
