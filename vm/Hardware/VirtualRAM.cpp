#include "VirtualRAM.hpp"

#include <Hypervisor/hv_vm.h>
#include <mach/vm_param.h>
#include <print>
#include <sys/mman.h>

#include "VMExtensions.hpp"

static constexpr hv_ipa_t GUEST_PHYSICAL_ADDRESS = 0x10000;

VirtualRAM::VirtualRAM(const size_t size) noexcept {
    void*const hostAddress = mmap(nullptr, size, PROT_READ, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (hostAddress == MAP_FAILED) {
        std::print("Unable to allocate memory for virtual RAM");
        exit(EXIT_FAILURE);
    }

    VM::guard(
        hv_vm_map(
            hostAddress,
            GUEST_PHYSICAL_ADDRESS,
            size,
            HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC
        ),
        "VM memory map"
    );

    this->_hostAddress = hostAddress;
}

VirtualRAM::~VirtualRAM() noexcept {
    VM::guard(
        hv_vm_unmap(GUEST_PHYSICAL_ADDRESS, PAGE_SIZE),
        "VM memory unmap"
    );

    if (munmap(this->_hostAddress, PAGE_SIZE) == -1) {
        std::print("Unable to deallocate memory for virtual RAM");
        exit(EXIT_FAILURE);
    }
}
