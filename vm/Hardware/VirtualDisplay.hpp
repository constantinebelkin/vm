#ifndef VIRTUAL_DISPLAY_H
#define VIRTUAL_DISPLAY_H

#include <cstddef>
#include <cstdint>

struct VirtualDisplay final {
    struct FrameBuffer final {
        size_t size;
        uint8_t* data;
    };

    struct Size final {
        uint16_t width;
        uint16_t height;
    };

    const struct Size size;

    VirtualDisplay() noexcept;
    ~VirtualDisplay() noexcept;

    struct FrameBuffer framebuffer(this const struct VirtualDisplay &self) noexcept;

private:
    void* _hostAddress;
};

#endif
