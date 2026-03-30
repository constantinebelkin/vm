#include "VirtualKeyboard.hpp"

#include <Hypervisor/hv_vm.h>
#include <Hypervisor/hv_vcpu.h>
#include <cstdint>
#include <mach/vm_param.h>
#include <print>
#include <sys/mman.h>

#include "VMExtensions.hpp"

static constexpr hv_ipa_t GUEST_PHYSICAL_ADDRESS = 0xC000;

VirtualKeyboard::VirtualKeyboard() {
    void*const hostAddress = mmap(nullptr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (hostAddress == MAP_FAILED) {
        std::print("Unable to allocate memory for virtual keyboard");
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

VirtualKeyboard::~VirtualKeyboard() {
    VM::guard(
        hv_vm_unmap(GUEST_PHYSICAL_ADDRESS, PAGE_SIZE),
        "VM memory unmap"
    );

    if (munmap(this->_hostAddress, PAGE_SIZE) == -1) {
        std::print("Unable to deallocate memory for virtual keyboard");
        exit(EXIT_FAILURE);
    }
}

void VirtualKeyboard::attachVCPU(this struct VirtualKeyboard &self, const uint64_t vcpu) noexcept {
    self._vcpu = vcpu;
}

bool VirtualKeyboard::hasPendingEvent(this const struct VirtualKeyboard &self) noexcept {
    const auto write_reg_address = reinterpret_cast<uintptr_t>(self._hostAddress);
    const auto read_reg_address = write_reg_address + sizeof(uint32_t);

    const auto keyboard_write_reg = reinterpret_cast<uint32_t*>(write_reg_address);
    const auto keyboard_read_reg = reinterpret_cast<uint32_t*>(read_reg_address);

    return *keyboard_write_reg != *keyboard_read_reg;
}

void VirtualKeyboard::keyDown(this struct VirtualKeyboard &self, const uint8_t key_code) noexcept {
//    if (self._vcpu == 0) {
//        return;
//    }

    const auto write_reg_address = reinterpret_cast<uintptr_t>(self._hostAddress);
    const auto buffer_address = write_reg_address + (sizeof(uint32_t) * 2);

    auto volatile buffer = reinterpret_cast<uint8_t*>(buffer_address);
    auto volatile keyboard_write_reg = reinterpret_cast<uint32_t*>(write_reg_address);

    uint32_t keyboard_write_reg_value = *keyboard_write_reg;
    buffer[keyboard_write_reg_value] = key_code;
    *keyboard_write_reg = ++keyboard_write_reg_value;

    hv_vcpus_exit(&self._vcpu, 1);
}
