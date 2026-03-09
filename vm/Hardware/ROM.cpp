#include "ROM.hpp"

#include <Hypervisor/hv_vm.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
    #include "VMOperationManager.h"
}

static void memory_safe_operation(kern_return_t memory_operation_result, const char description[]) {
    switch (memory_operation_result) {
        case KERN_SUCCESS:
            printf("Memory operation successfull: %s\n", description);
            break;

        case KERN_INVALID_ADDRESS:
            printf("Memory operation failed with invalid address provided: %s\n", description);
            exit(EXIT_FAILURE);

        case KERN_PROTECTION_FAILURE:
            printf("Memory operation failed with protection failure: %s\n", description);
            exit(EXIT_FAILURE);

        default:
            printf("Memory operation failed with error code %d: %s\n", memory_operation_result, description);
            exit(EXIT_FAILURE);
    }
}

struct ROM::Impl {
    void * physical_address;
};

ROM::ROM(const char firmware_path[]) {
    const int payload_file_descriptor = open(firmware_path, O_RDONLY);
    if (payload_file_descriptor < 0) {
        printf("Unable to open file payload with error #%d", errno);
        exit(EXIT_FAILURE);
    }

    void *const payload_buffer = malloc(PAGE_SIZE);
    const ssize_t payload_size = read(payload_file_descriptor, payload_buffer, PAGE_SIZE);
    close(payload_file_descriptor);

    if (payload_size < 4) {
        printf("Invalid payload content");
        exit(EXIT_FAILURE);
    }

    this->_impl = new Impl();

    memory_safe_operation(
        mach_vm_allocate(
            current_task(),
            (mach_vm_address_t *)&this->_impl->physical_address,
            PAGE_SIZE,
            VM_FLAGS_ANYWHERE
        ),
        "Memory allocate"
    );

    memcpy(this->_impl->physical_address, payload_buffer, (size_t)payload_size);

    vm_safe_operation(
        hv_vm_map(
            this->_impl->physical_address,
            (hv_ipa_t)ROM_ADDRESS,
            PAGE_SIZE,
            HV_MEMORY_READ | HV_MEMORY_EXEC
        ),
        "VM memory map"
    );
}

ROM::~ROM() {
    vm_safe_operation(
        hv_vm_unmap((hv_ipa_t)ROM_ADDRESS, PAGE_SIZE),
        "VM memory unmap"
    );

    memory_safe_operation(
        mach_vm_deallocate(
            current_task(),
            (mach_vm_address_t)this->_impl->physical_address,
            PAGE_SIZE
        ),
        "Memory deallocate"
    );

    delete this->_impl;
}
