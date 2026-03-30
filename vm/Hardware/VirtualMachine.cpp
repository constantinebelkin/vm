#include "VirtualMachine.hpp"

#include <Hypervisor/hv_vcpu.h>
#include <Hypervisor/hv_vm.h>
#include <Hypervisor/hv_gic.h>
#include <Hypervisor/hv_gic_config.h>
#include <mach/mach_vm.h>
#include <print>
#include <thread>

#include "VirtualDisplay.hpp"
#include "VirtualKeyboard.hpp"
#include "VirtualRAM.hpp"
#include "VirtualROM.hpp"
#include "VMExtensions.hpp"

struct VirtualMachine::Impl {
    struct VCPU {
        hv_vcpu_t handle;
        std::jthread thread;
    };

    VCPU vcpu[VCPU_COUNT];

    bool forceQuitRequested = false;

    const struct VirtualRAM ram = VirtualRAM(PAGE_SIZE*2);
    const struct VirtualROM rom;

    explicit Impl(VirtualMachine& parent, const char firmwarePath[]) noexcept :
    _parent(parent),
    rom(VirtualROM(firmwarePath)) {}

    void handleException(this const struct Impl &self, const uint64_t exception_code, const hv_vcpu_t vcpu) noexcept {
        constexpr uint32_t EXCEPTION_CLASS_MASK = 0xFC000000;

//        constexpr uint32_t EXCEPTION_CLASS_WFI = 0x4000000;
        constexpr uint32_t EXCEPTION_CLASS_HVC = 0x58000000;

        const uint32_t exception_class = static_cast<uint32_t>(exception_code) & EXCEPTION_CLASS_MASK;

        if (exception_class == EXCEPTION_CLASS_HVC) {
            const auto hvc_opcode = static_cast<uint16_t>(exception_code);
            if (hvc_opcode == 0xDEB) {
                self.debug(vcpu);
            } else {
                std::println("Unknown HVC opcode: 0x{:x}", hvc_opcode);
                self.debug(vcpu);
            }
        } else {
            std::println("Unknown exception class: 0x{:x}", exception_class);
            self.debug(vcpu);
            exit(EXIT_FAILURE);
        }
    }

    void handleCancel(this struct Impl &self, const hv_vcpu_t vcpu) noexcept {
        const bool keyboardEventPending = self._parent.keyboard->hasPendingEvent();
        if (keyboardEventPending) {
            hv_vcpu_set_pending_interrupt(vcpu, HV_INTERRUPT_TYPE_IRQ, 0x1);
        } else {
            self.forceQuitRequested = true;
        }
    }

    void debug(this const struct Impl &self, hv_vcpu_t vcpu) {
        uint64_t esr1, far1, elr1, fp, lr, sp, pc, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
        hv_vcpu_get_reg(vcpu, HV_REG_X0, &x0);
        hv_vcpu_get_reg(vcpu, HV_REG_X1, &x1);
        hv_vcpu_get_reg(vcpu, HV_REG_X2, &x2);
        hv_vcpu_get_reg(vcpu, HV_REG_X3, &x3);
        hv_vcpu_get_reg(vcpu, HV_REG_X4, &x4);
        hv_vcpu_get_reg(vcpu, HV_REG_X5, &x5);
        hv_vcpu_get_reg(vcpu, HV_REG_X6, &x6);
        hv_vcpu_get_reg(vcpu, HV_REG_X7, &x7);
        hv_vcpu_get_reg(vcpu, HV_REG_X8, &x8);
        hv_vcpu_get_reg(vcpu, HV_REG_X9, &x9);
        hv_vcpu_get_reg(vcpu, HV_REG_PC, &pc);
        hv_vcpu_get_reg(vcpu, HV_REG_X29, &fp);
        hv_vcpu_get_reg(vcpu, HV_REG_X30, &lr);
        hv_vcpu_get_sys_reg(vcpu, HV_SYS_REG_SP_EL1, &sp);
        hv_vcpu_get_sys_reg(vcpu, HV_SYS_REG_FAR_EL1, &far1);
        hv_vcpu_get_sys_reg(vcpu, HV_SYS_REG_ESR_EL1, &esr1);
        hv_vcpu_get_sys_reg(vcpu, HV_SYS_REG_ESR_EL1, &far1);
        hv_vcpu_get_sys_reg(vcpu, HV_SYS_REG_ELR_EL1, &elr1);

        std::println("X0: 0x{:x}", x0);
        std::println("X1: 0x{:x}", x1);
        std::println("X2: 0x{:x}", x2);
        std::println("X3: 0x{:x}", x3);
        std::println("X4: 0x{:x}", x4);
        std::println("X5: 0x{:x}", x5);
        std::println("X6: 0x{:x}", x6);
        std::println("X7: 0x{:x}", x7);
        std::println("X8: 0x{:x}", x8);
        std::println("X9: 0x{:x}", x9);
        std::println("PC: 0x{:x}", pc);
        std::println("SP: 0x{:x}", sp);
        std::println("FP: 0x{:x}", fp);
        std::println("LR: 0x{:x}", lr);
        std::println("ESR_EL1: 0x{:x}", esr1);
        std::println("FAR_EL1: 0x{:x}", far1);
        std::println("ELR_EL1: 0x{:x}", elr1);
    }

private:
    VirtualMachine& _parent;
};

std::unique_ptr<struct VirtualMachine> VirtualMachine::create(const char firmwarePath[]) noexcept {
    VM::guard(hv_vm_create(NULL), "VM create");
    
    hv_gic_config_t config = hv_gic_config_create();

    // 2. Задаем адрес для Distributor (обычно требует выравнивания 64КБ)
    // Пусть будет 0x08000000
    hv_ipa_t dist_addr = 0x08000000;
    VM::guard(hv_gic_config_set_distributor_base(config, dist_addr), "GIC Dist base");

    // 3. Задаем адрес для Redistributor (следом за дистрибьютором)
    // Каждому vCPU нужно 128КБ. Если у тебя 1 CPU, 0x08000000 + 64КБ хватит.
    hv_ipa_t redist_addr = 0x08010000;
    VM::guard(hv_gic_config_set_redistributor_base(config, redist_addr), "GIC Redist base");
    
    VM::guard(hv_gic_create(config), "GIC setup");

    return std::unique_ptr<struct VirtualMachine>(new VirtualMachine(firmwarePath));
}

VirtualMachine::VirtualMachine(const char firmwarePath[]) noexcept :
_pImpl(std::make_unique<VirtualMachine::Impl>(*this, firmwarePath)),
display(std::make_unique<VirtualDisplay>()),
keyboard(std::make_unique<struct VirtualKeyboard>()) {}

VirtualMachine::~VirtualMachine() noexcept {
    VM::guard(hv_vm_destroy(), "VM destroy");
}

void VirtualMachine::run(this struct VirtualMachine &self) noexcept {
    self._pImpl->vcpu[VirtualMachine::MAIN_CPU].thread = std::jthread([&self] {
        hv_vcpu_t main_vcpu;
        hv_vcpu_exit_t *cpu_exit;

        VM::guard(hv_vcpu_create(&main_vcpu, &cpu_exit, NULL), "vCPU create");
        self._pImpl->vcpu[VirtualMachine::MAIN_CPU].handle = main_vcpu;

        self.keyboard->attachVCPU(main_vcpu);

        VM::guard(
            hv_vcpu_set_reg(main_vcpu, HV_REG_CPSR, 0b101),
            "Init SPSR_EL1 register"
        );

        hv_gic_set_icc_reg(main_vcpu, HV_GIC_ICC_REG_SRE_EL1, 1);
        hv_gic_set_icc_reg(main_vcpu, HV_GIC_ICC_REG_IGRPEN1_EL1, 1);

        do {
            hv_vcpu_run(main_vcpu);

            switch (cpu_exit->reason) {
                case HV_EXIT_REASON_CANCELED:
                    self._pImpl->handleCancel(main_vcpu);
                    break;

                case HV_EXIT_REASON_EXCEPTION:
                    std::print("HV_EXIT_REASON_EXCEPTION = 0x{:x}\n", cpu_exit->exception.syndrome);
                    self._pImpl->handleException(cpu_exit->exception.syndrome, main_vcpu);
                    break;

                case HV_EXIT_REASON_VTIMER_ACTIVATED:
                    std::print("HV_EXIT_REASON_VTIMER_ACTIVATED\n");
                    break;

                case HV_EXIT_REASON_UNKNOWN:
                    std::print("HV_EXIT_REASON_UNKNOWN\n");
                    break;
            }
        } while (!self._pImpl->forceQuitRequested);

        VM::guard(hv_vcpu_destroy(main_vcpu), "vCPU destroy");
    });
}
