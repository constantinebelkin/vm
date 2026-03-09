#include "VMOperationManager.h"

#include <stdio.h>
#include <stdlib.h>

void vm_safe_operation(hv_return_t vm_operation_result, const char description[]) {
    switch (vm_operation_result) {
        case HV_SUCCESS:
            printf("VM operation executed successfuly: %s\n", description);
            break;

        case HV_ERROR:
            printf("VM operation failed with unknown error: %s\n", description);
            exit(EXIT_FAILURE);

        case HV_BUSY:
            printf("VM operation failed becuase hypervisor is busy: %s\n", description);
            exit(EXIT_FAILURE);

        default:
            printf("VM operation failed with error code %d: %s\n", vm_operation_result, description);
            exit(EXIT_FAILURE);

        // TODO: Handle other cases
    }
}
