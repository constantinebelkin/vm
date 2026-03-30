#ifndef VIRTUAL_RAM_H
#define VIRTUAL_RAM_H

#include <cstddef>

struct VirtualRAM final {
    VirtualRAM(const size_t size) noexcept;
    ~VirtualRAM() noexcept;

private:
    void* _hostAddress;
};

#endif
