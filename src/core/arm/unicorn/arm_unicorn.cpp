// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <unicorn/arm64.h>
#include "common/assert.h"
#include "common/microprofile.h"
#include "core/arm/cpu_interrupt_handler.h"
#include "core/arm/unicorn/arm_unicorn.h"
#include "core/core.h"
#include "core/core_timing.h"
#include "core/hle/kernel/scheduler.h"
#include "core/hle/kernel/svc.h"
#include "core/memory.h"

namespace Core {

// Load Unicorn DLL once on Windows using RAII
#ifdef _MSC_VER
#include <unicorn_dynload.h>
struct LoadDll {
private:
    LoadDll() {
        ASSERT(uc_dyn_load(NULL, 0));
    }
    ~LoadDll() {
        ASSERT(uc_dyn_free());
    }
    static LoadDll g_load_dll;
};
LoadDll LoadDll::g_load_dll;
#endif

#define CHECKED(expr)                                                                              \
    do {                                                                                           \
        if (auto _cerr = (expr)) {                                                                 \
            ASSERT_MSG(false, "Call " #expr " failed with error: {} ({})\n", _cerr,                \
                       uc_strerror(_cerr));                                                        \
        }                                                                                          \
    } while (0)

static void CodeHook(uc_engine* uc, uint64_t address, uint32_t size, void* user_data) {
    GDBStub::BreakpointAddress bkpt =
        GDBStub::GetNextBreakpointFromAddress(address, GDBStub::BreakpointType::Execute);
    if (GDBStub::IsMemoryBreak() ||
        (bkpt.type != GDBStub::BreakpointType::None && address == bkpt.address)) {
        auto core = static_cast<ARM_Unicorn*>(user_data);
        core->RecordBreak(bkpt);
        uc_emu_stop(uc);
    }
}

static bool UnmappedMemoryHook(uc_engine* uc, uc_mem_type type, u64 addr, int size, u64 value,
                               void* user_data) {
    auto* const system = static_cast<System*>(user_data);

    ARM_Interface::ThreadContext64 ctx{};
    system->CurrentArmInterface().SaveContext(ctx);
    ASSERT_MSG(false, "Attempted to read from unmapped memory: 0x{:X}, pc=0x{:X}, lr=0x{:X}", addr,
               ctx.pc, ctx.cpu_registers[30]);

    return false;
}

ARM_Unicorn::ARM_Unicorn(System& system, CPUInterrupts& interrupt_handlers, bool uses_wall_clock,
                         Arch architecture, std::size_t core_index)
    : ARM_Interface{system, interrupt_handlers, uses_wall_clock}, core_index{core_index} {
    const auto arch = architecture == Arch::AArch32 ? UC_ARCH_ARM : UC_ARCH_ARM64;
    CHECKED(uc_open(arch, UC_MODE_ARM, &uc));

    auto fpv = 3 << 20;
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_CPACR_EL1, &fpv));

    uc_hook hook{};
    CHECKED(uc_hook_add(uc, &hook, UC_HOOK_INTR, (void*)InterruptHook, this, 0, UINT64_MAX));
    CHECKED(uc_hook_add(uc, &hook, UC_HOOK_MEM_INVALID, (void*)UnmappedMemoryHook, &system, 0,
                        UINT64_MAX));
    if (GDBStub::IsServerEnabled()) {
        CHECKED(uc_hook_add(uc, &hook, UC_HOOK_CODE, (void*)CodeHook, this, 0, UINT64_MAX));
        last_bkpt_hit = false;
    }
}

ARM_Unicorn::~ARM_Unicorn() {
    CHECKED(uc_close(uc));
}

void ARM_Unicorn::SetPC(u64 pc) {
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_PC, &pc));
}

u64 ARM_Unicorn::GetPC() const {
    u64 val{};
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_PC, &val));
    return val;
}

u64 ARM_Unicorn::GetReg(int regn) const {
    u64 val{};
    auto treg = UC_ARM64_REG_SP;
    if (regn <= 28) {
        treg = (uc_arm64_reg)(UC_ARM64_REG_X0 + regn);
    } else if (regn < 31) {
        treg = (uc_arm64_reg)(UC_ARM64_REG_X29 + regn - 29);
    }
    CHECKED(uc_reg_read(uc, treg, &val));
    return val;
}

void ARM_Unicorn::SetReg(int regn, u64 val) {
    auto treg = UC_ARM64_REG_SP;
    if (regn <= 28) {
        treg = (uc_arm64_reg)(UC_ARM64_REG_X0 + regn);
    } else if (regn < 31) {
        treg = (uc_arm64_reg)(UC_ARM64_REG_X29 + regn - 29);
    }
    CHECKED(uc_reg_write(uc, treg, &val));
}

u128 ARM_Unicorn::GetVectorReg(int /*index*/) const {
    UNIMPLEMENTED();
    static constexpr u128 res{};
    return res;
}

void ARM_Unicorn::SetVectorReg(int /*index*/, u128 /*value*/) {
    UNIMPLEMENTED();
}

u32 ARM_Unicorn::GetPSTATE() const {
    u64 nzcv{};
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_NZCV, &nzcv));
    return static_cast<u32>(nzcv);
}

void ARM_Unicorn::SetPSTATE(u32 pstate) {
    u64 nzcv = pstate;
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_NZCV, &nzcv));
}

VAddr ARM_Unicorn::GetTlsAddress() const {
    u64 base{};
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_TPIDRRO_EL0, &base));
    return base;
}

void ARM_Unicorn::SetTlsAddress(VAddr base) {
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_TPIDRRO_EL0, &base));
}

u64 ARM_Unicorn::GetTPIDR_EL0() const {
    u64 value{};
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_TPIDR_EL0, &value));
    return value;
}

void ARM_Unicorn::SetTPIDR_EL0(u64 value) {
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_TPIDR_EL0, &value));
}

void ARM_Unicorn::ChangeProcessorID(std::size_t new_core_id) {
    core_index = new_core_id;
}

void ARM_Unicorn::Run() {
    if (GDBStub::IsServerEnabled()) {
        ExecuteInstructions(std::max(4000000U, 0U));
    } else {
        while (true) {
            if (interrupt_handlers[core_index].IsInterrupted()) {
                return;
            }
            ExecuteInstructions(10);
        }
    }
}

void ARM_Unicorn::Step() {
    ExecuteInstructions(1);
}

MICROPROFILE_DEFINE(ARM_Jit_Unicorn, "ARM JIT", "Unicorn", MP_RGB(255, 64, 64));

void ARM_Unicorn::ExecuteInstructions(std::size_t num_instructions) {
    MICROPROFILE_SCOPE(ARM_Jit_Unicorn);

    // Temporarily map the code page for Unicorn
    u64 map_addr{GetPC() & ~Memory::PAGE_MASK};
    std::vector<u8> page_buffer(Memory::PAGE_SIZE);
    system.Memory().ReadBlock(map_addr, page_buffer.data(), page_buffer.size());

    CHECKED(uc_mem_map_ptr(uc, map_addr, page_buffer.size(),
                           UC_PROT_READ | UC_PROT_WRITE | UC_PROT_EXEC, page_buffer.data()));
    CHECKED(uc_emu_start(uc, GetPC(), 1ULL << 63, 0, num_instructions));
    CHECKED(uc_mem_unmap(uc, map_addr, page_buffer.size()));
    if (GDBStub::IsServerEnabled()) {
        if (last_bkpt_hit && last_bkpt.type == GDBStub::BreakpointType::Execute) {
            uc_reg_write(uc, UC_ARM64_REG_PC, &last_bkpt.address);
        }

        Kernel::Thread* const thread = system.CurrentScheduler().GetCurrentThread();
        SaveContext(thread->GetContext64());
        if (last_bkpt_hit || GDBStub::IsMemoryBreak() || GDBStub::GetCpuStepFlag()) {
            last_bkpt_hit = false;
            GDBStub::Break();
            GDBStub::SendTrap(thread, 5);
        }
    }
}

void ARM_Unicorn::SaveContext(ThreadContext64& ctx) {
    int uregs[32];
    void* tregs[32];

    CHECKED(uc_reg_read(uc, UC_ARM64_REG_SP, &ctx.sp));
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_PC, &ctx.pc));
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_NZCV, &ctx.pstate));

    for (auto i = 0; i < 29; ++i) {
        uregs[i] = UC_ARM64_REG_X0 + i;
        tregs[i] = &ctx.cpu_registers[i];
    }
    uregs[29] = UC_ARM64_REG_X29;
    tregs[29] = (void*)&ctx.cpu_registers[29];
    uregs[30] = UC_ARM64_REG_X30;
    tregs[30] = (void*)&ctx.cpu_registers[30];

    CHECKED(uc_reg_read_batch(uc, uregs, tregs, 31));

    for (int i = 0; i < 32; ++i) {
        uregs[i] = UC_ARM64_REG_Q0 + i;
        tregs[i] = &ctx.vector_registers[i];
    }

    CHECKED(uc_reg_read_batch(uc, uregs, tregs, 32));
}

void ARM_Unicorn::LoadContext(const ThreadContext64& ctx) {
    int uregs[32];
    void* tregs[32];

    CHECKED(uc_reg_write(uc, UC_ARM64_REG_SP, &ctx.sp));
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_PC, &ctx.pc));
    CHECKED(uc_reg_write(uc, UC_ARM64_REG_NZCV, &ctx.pstate));

    for (int i = 0; i < 29; ++i) {
        uregs[i] = UC_ARM64_REG_X0 + i;
        tregs[i] = (void*)&ctx.cpu_registers[i];
    }
    uregs[29] = UC_ARM64_REG_X29;
    tregs[29] = (void*)&ctx.cpu_registers[29];
    uregs[30] = UC_ARM64_REG_X30;
    tregs[30] = (void*)&ctx.cpu_registers[30];

    CHECKED(uc_reg_write_batch(uc, uregs, tregs, 31));

    for (auto i = 0; i < 32; ++i) {
        uregs[i] = UC_ARM64_REG_Q0 + i;
        tregs[i] = (void*)&ctx.vector_registers[i];
    }

    CHECKED(uc_reg_write_batch(uc, uregs, tregs, 32));
}

void ARM_Unicorn::PrepareReschedule() {
    CHECKED(uc_emu_stop(uc));
}

void ARM_Unicorn::ClearExclusiveState() {}

void ARM_Unicorn::ClearInstructionCache() {}

void ARM_Unicorn::RecordBreak(GDBStub::BreakpointAddress bkpt) {
    last_bkpt = bkpt;
    last_bkpt_hit = true;
}

void ARM_Unicorn::InterruptHook(uc_engine* uc, u32 int_no, void* user_data) {
    u32 esr{};
    CHECKED(uc_reg_read(uc, UC_ARM64_REG_ESR, &esr));

    const auto ec = esr >> 26;
    const auto iss = esr & 0xFFFFFF;

    auto* const arm_instance = static_cast<ARM_Unicorn*>(user_data);

    switch (ec) {
    case 0x15: // SVC
        Kernel::Svc::Call(arm_instance->system, iss);
        break;
    }
}

} // namespace Core
