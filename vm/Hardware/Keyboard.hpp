#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

struct Keyboard {
};

void keyboard_key_down(struct Keyboard *const self, const uint8_t key_code);

#endif
