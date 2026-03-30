#ifndef VM_OPERATION_MANAGER_H
#define VM_OPERATION_MANAGER_H

#include <arm64/hv/hv_kern_types.h>

namespace VM {

void guard(hv_return_t vm_operation_result, const char description[]);

}

#endif
