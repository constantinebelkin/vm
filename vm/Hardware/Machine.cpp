#include "Machine.hpp"

#include <Hypervisor/hv_vcpu.h>
#include <Hypervisor/hv_vm.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "Display.hpp"
#include "ROM.hpp"

extern "C" {
    #include "VMOperationManager.h"
}

struct Machine::Impl {
    uint64_t vcpus[VCPU_NUMBER];
};

static void handle_exception(uint64_t exception_code, hv_vcpu_t vcpu) {
    #define EXCEPTION_CLASS_MASK 0xFC000000
    #define EXCEPTION_CLASS_HVC 0x58000000

    const uint32_t exception_class = (uint32_t)exception_code & EXCEPTION_CLASS_MASK;
    if (exception_class == EXCEPTION_CLASS_HVC) {
        const uint16_t hvc_opcode = (uint16_t)exception_code;
        if (hvc_opcode == 0xDEB) {
            uint64_t pc, x2;
            hv_vcpu_get_reg(vcpu, HV_REG_X2, &x2);
            hv_vcpu_get_reg(vcpu, HV_REG_PC, &pc);

            printf("X2: 0x%llx\n", x2);
            printf("PC: 0x%llx\n", pc);
        } else {
            printf("Unknown HVC opcode: 0x%x", hvc_opcode);
        }
    } else {
        printf("Unknown exception class: 0x%x", exception_class);
    }
}

static void * cpu_run(void *_Nullable arg) {
    auto self = (struct Machine *const)arg;

    hv_vcpu_t main_cpu;
    hv_vcpu_exit_t *cpu_exit;

    vm_safe_operation(hv_vcpu_create(&main_cpu, &cpu_exit, NULL), "vCPU create");

    self->_impl->vcpus[0] = main_cpu;

    vm_safe_operation(
        hv_vcpu_set_reg(main_cpu, HV_REG_PC, ROM_ADDRESS),
        "Init PC register"
    );

    vm_safe_operation(
        hv_vcpu_set_reg(main_cpu, HV_REG_CPSR, 0b101),
        "Init SPSR_EL1 register"
    );

    vm_safe_operation(hv_vcpu_run(main_cpu), "vCPU run");

    switch (cpu_exit->reason) {
        case HV_EXIT_REASON_CANCELED:
            printf("HV_EXIT_REASON_CANCELED\n");
            break;

        case HV_EXIT_REASON_EXCEPTION:
            printf("HV_EXIT_REASON_EXCEPTION = 0x%llx\n", cpu_exit->exception.syndrome);
            handle_exception(cpu_exit->exception.syndrome, main_cpu);
            break;

        case HV_EXIT_REASON_VTIMER_ACTIVATED:
            printf("HV_EXIT_REASON_VTIMER_ACTIVATED\n");
            break;

        case HV_EXIT_REASON_UNKNOWN:
            printf("HV_EXIT_REASON_UNKNOWN\n");
            break;
    }

    vm_safe_operation(hv_vcpu_destroy(main_cpu), "vCPU destroy");

    return NULL;
}

Machine::Machine(const char firmware_path[]) {
    this->_impl = new Impl();

    vm_safe_operation(hv_vm_create(NULL), "VM create");

    this->rom = new ROM(firmware_path);
    this->display = new Display();
}

Machine::~Machine() {
    delete this->display;
    delete this->rom;

    delete this->_impl;

    vm_safe_operation(hv_vm_destroy(), "VM destroy");
}

void Machine::run() {
    pthread_t underlying_thread_id;
    if (pthread_create(&underlying_thread_id, NULL, cpu_run, (void *)this) != 0) {
        printf("Unable to create thread for CPU.\n");
        exit(EXIT_FAILURE);
    }
}
