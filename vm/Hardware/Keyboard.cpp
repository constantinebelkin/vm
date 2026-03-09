#include "Keyboard.hpp"
#include <stdio.h>

void keyboard_key_down(struct Keyboard *const self, const uint8_t key_code) {
    printf("%d", key_code);
}
