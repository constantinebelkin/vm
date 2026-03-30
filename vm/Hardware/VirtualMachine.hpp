#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <cstdint>
#include <memory>

struct VirtualDisplay;
struct VirtualKeyboard;

struct VirtualMachine final {
    static constexpr uint8_t VCPU_COUNT = 1;
    static constexpr uint8_t MAIN_CPU = 0;

    const std::unique_ptr<struct VirtualDisplay> display;
    const std::unique_ptr<struct VirtualKeyboard> keyboard;

    std::unique_ptr<struct VirtualMachine> static create(const char firmwarePath[]) noexcept;
    ~VirtualMachine() noexcept;

    void run(this struct VirtualMachine &self) noexcept;

private:
    VirtualMachine(const char firmwarePath[]) noexcept;

    struct Impl;
    std::unique_ptr<struct Impl> _pImpl;
};

#endif
