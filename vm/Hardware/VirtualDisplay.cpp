#include "VirtualDisplay.hpp"

#include <Hypervisor/hv_vm.h>
#include <mach/vm_param.h>
#include <print>
#include <sys/mman.h>

#include "VMExtensions.hpp"

static constexpr hv_ipa_t GUEST_PHYSICAL_ADDRESS = 0x4000;

VirtualDisplay::VirtualDisplay() noexcept :
size({.width = 80, .height = 25})
{
    void*const hostAddress = mmap(nullptr, PAGE_SIZE, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (hostAddress == MAP_FAILED) {
        std::print("Unable to allocate memory for virtual display");
        exit(EXIT_FAILURE);
    }

    VM::guard(
        hv_vm_map(
            hostAddress,
            GUEST_PHYSICAL_ADDRESS,
            PAGE_SIZE,
            HV_MEMORY_READ | HV_MEMORY_WRITE
        ),
        "VM memory map"
    );

    this->_hostAddress = hostAddress;
}

VirtualDisplay::~VirtualDisplay() noexcept {
    VM::guard(
        hv_vm_unmap(GUEST_PHYSICAL_ADDRESS, PAGE_SIZE),
        "VM memory unmap"
    );

    if (munmap(this->_hostAddress, PAGE_SIZE) == -1) {
        std::print("Unable to deallocate memory for virtual display");
        exit(EXIT_FAILURE);
    }
}

struct VirtualDisplay::FrameBuffer VirtualDisplay::framebuffer(this const struct VirtualDisplay &self) noexcept {
    return {
        .size = (self.size.width * self.size.height) + sizeof(uint8_t),
        .data = static_cast<uint8_t*>(self._hostAddress)
    };
}
