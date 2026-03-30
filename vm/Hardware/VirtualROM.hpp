#ifndef VIRTUAL_ROM_H
#define VIRTUAL_ROM_H

#include <cstdint>
#include <memory>

struct VirtualROM final {
    explicit VirtualROM(const char firmwarePath[]) noexcept;
    ~VirtualROM() noexcept;

private:
    void* _hostAddress;
};

#endif
