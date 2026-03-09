#import "DisplayObserverAdapter.h"

DisplayObserverAdapter::DisplayObserverAdapter(const id<DisplayObserver> impl) : objcImpl(impl) {}

void DisplayObserverAdapter::display_did_change_buffer_state(struct Display *const display) {
    [this->objcImpl displayDidChangeBufferState:display];
}
