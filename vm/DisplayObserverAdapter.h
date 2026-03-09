#import "Display.hpp"

@protocol DisplayObserver

- (void)displayDidChangeBufferState:(struct Display *const)display;

@end

struct DisplayObserverAdapter final : public IDisplayObserver {
    DisplayObserverAdapter(const id<DisplayObserver> impl);

    void display_did_change_buffer_state(struct Display *const display);

private:
    __weak id<DisplayObserver> objcImpl;
};
