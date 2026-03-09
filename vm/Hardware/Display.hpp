#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

enum DisplayMode {
    DisplayModeVGA,
    DisplayModeHD
};

struct DisplayDimensions {
    uint16_t width;
    uint16_t height;
};

struct IDisplayObserver {
    virtual void display_did_change_buffer_state(struct Display *const display) = 0;
};

struct Display final {
    Display();
    ~Display();

    struct DisplayDimensions get_dimensions();
    uint8_t * get_framebuffer();

    void print_characters(const uint8_t characters[], const uint64_t count);

    void add_observer(IDisplayObserver *const observer);
    void remove_observer(IDisplayObserver *const observer);

private:
    struct Impl;
    struct Impl *_impl;
};

#endif
