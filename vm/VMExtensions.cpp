#include "VMExtensions.hpp"

#include <print>
#include <stdlib.h>

void VM::guard(hv_return_t vm_operation_result, const char description[]) {
    switch (vm_operation_result) {
        case HV_SUCCESS:
            std::print("VM operation executed successfuly: {}\n", description);
            break;

        case HV_ERROR:
            std::print("VM operation failed with unknown error: {}\n", description);
            exit(EXIT_FAILURE);

        case HV_BUSY:
            std::print("VM operation failed becuase hypervisor is busy: {}\n", description);
            exit(EXIT_FAILURE);

        default:
            std::print("VM operation failed with error code {}: {}\n", vm_operation_result, description);
            exit(EXIT_FAILURE);

        // TODO: Handle other cases
    }
}
