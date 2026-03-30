#include "VirtualROM.hpp"

#include <Hypervisor/hv_vm.h>
#include <fcntl.h>
#include <mach/vm_param.h>
#include <print>
#include <sys/mman.h>
#include <unistd.h>

#include "VMExtensions.hpp"

static constexpr hv_ipa_t GUEST_PHYSICAL_ADDRESS = 0x0;

VirtualROM::VirtualROM(const char firmwarePath[]) noexcept {
    const int firmwareFileDescriptor = open(firmwarePath, O_RDONLY);
    if (firmwareFileDescriptor < 0) {
        std::print("Unable to open file payload with error #%d", errno);
        exit(EXIT_FAILURE);
    }

    void*const hostAddress = mmap(nullptr, PAGE_SIZE, PROT_READ, MAP_PRIVATE, firmwareFileDescriptor, 0);
    if (hostAddress == MAP_FAILED) {
        std::print("Unable to allocate memory for virtual ROM");
        exit(EXIT_FAILURE);
    }

    VM::guard(
        hv_vm_map(
            hostAddress,
            GUEST_PHYSICAL_ADDRESS,
            PAGE_SIZE,
            HV_MEMORY_READ | HV_MEMORY_EXEC
        ),
        "VM memory map"
    );

    close(firmwareFileDescriptor);

    this->_hostAddress = hostAddress;
}

VirtualROM::~VirtualROM() noexcept {
    VM::guard(
        hv_vm_unmap(GUEST_PHYSICAL_ADDRESS, PAGE_SIZE),
        "VM memory unmap"
    );

    if (munmap(this->_hostAddress, PAGE_SIZE) == -1) {
        std::print("Unable to deallocate memory for virtual ROM");
        exit(EXIT_FAILURE);
    }
}
