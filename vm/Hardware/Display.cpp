#include "Display.hpp"

#include <vector>

struct Display::Impl {
    std::vector<struct IDisplayObserver *> observers;
    uint8_t *framebuffer;
    uint16_t current_position;
};

Display::Display() {
    this->_impl = new Impl();
    this->_impl->observers = std::vector<struct IDisplayObserver *>();
    this->_impl->framebuffer = (uint8_t *)calloc(80*25, sizeof(uint8_t));
    this->_impl->current_position = 0;
}

Display::~Display() {
    this->_impl->observers.clear();

    delete this->_impl;
}

uint8_t * Display::get_framebuffer() {
    return this->_impl->framebuffer;
}

void Display::print_characters(const uint8_t characters[], const uint64_t count) {
    uint16_t current_position = this->_impl->current_position;
    for (uint64_t char_index = 0; char_index < count; char_index++) {
        if (current_position >= 2000) {
            memcpy(this->_impl->framebuffer, &this->_impl->framebuffer[80], 2000-80);
            memset(&this->_impl->framebuffer[2000-81], 0x0, 80);
            current_position -= 80;
        }

        const uint8_t character = characters[char_index];
        switch (character) {
            case 0x8:
            case 0x7F: {
                if (current_position > 0) {
                    this->_impl->framebuffer[current_position] = 0x0;
                    current_position--;
                    this->_impl->framebuffer[current_position] = 0x0;
                    while (current_position > 1 && this->_impl->framebuffer[current_position-1] == 0x0) {
                        current_position--;
                    }
                }
            }
            break;
            case '\r': {
                this->_impl->framebuffer[current_position] = 0x0;
                current_position = ((current_position / 80) * 80) + (current_position + (80 - current_position));
            }
            break;
            default:
                this->_impl->framebuffer[current_position] = character;
                current_position++;
        }
    }

    this->_impl->framebuffer[current_position] = '|';
    this->_impl->current_position = current_position;

    for (struct IDisplayObserver * observer : this->_impl->observers) {
        observer->display_did_change_buffer_state(this);
    }
}

void Display::add_observer(struct IDisplayObserver *const observer) {
    this->_impl->observers.push_back(observer);
}

void Display::remove_observer(struct IDisplayObserver *const observer) {
    std::erase(this->_impl->observers, observer);
}
