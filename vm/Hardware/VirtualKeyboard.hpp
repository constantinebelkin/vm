#ifndef VIRTUAL_KEYBOARD_H
#define VIRTUAL_KEYBOARD_H

#include <condition_variable>
#include <cstdint>

struct KeyboardObject;

struct VirtualKeyboard final {
    std::mutex lock;
    std::condition_variable interruptionCondition;

    VirtualKeyboard();
    ~VirtualKeyboard();

    void attachVCPU(this struct VirtualKeyboard &self, const uint64_t vcpu) noexcept;
    bool hasPendingEvent(this const struct VirtualKeyboard &self) noexcept;
    void keyDown(this struct VirtualKeyboard &self, const uint8_t key_code) noexcept;

private:
    void* _hostAddress;
    uint64_t _vcpu;
};

#endif
