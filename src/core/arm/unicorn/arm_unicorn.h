// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <unicorn/unicorn.h>
#include "common/common_types.h"
#include "core/arm/arm_interface.h"
#include "core/gdbstub/gdbstub.h"

namespace Core {

class System;

class ARM_Unicorn final : public ARM_Interface {
public:
    enum class Arch {
        AArch32, // 32-bit ARM
        AArch64, // 64-bit ARM
    };

    explicit ARM_Unicorn(System& system, CPUInterrupts& interrupt_handlers, bool uses_wall_clock,
                         Arch architecture, std::size_t core_index);
    ~ARM_Unicorn() override;

    void SetPC(u64 pc) override;
    u64 GetPC() const override;
    u64 GetReg(int index) const override;
    void SetReg(int index, u64 value) override;
    u128 GetVectorReg(int index) const override;
    void SetVectorReg(int index, u128 value) override;
    u32 GetPSTATE() const override;
    void SetPSTATE(u32 pstate) override;
    VAddr GetTlsAddress() const override;
    void SetTlsAddress(VAddr address) override;
    void SetTPIDR_EL0(u64 value) override;
    u64 GetTPIDR_EL0() const override;
    void ChangeProcessorID(std::size_t new_core_id) override;
    void PrepareReschedule() override;
    void ClearExclusiveState() override;
    void ExecuteInstructions(std::size_t num_instructions);
    void Run() override;
    void Step() override;
    void ClearInstructionCache() override;
    void PageTableChanged(Common::PageTable&, std::size_t) override {}
    void RecordBreak(GDBStub::BreakpointAddress bkpt);

    void SaveContext(ThreadContext32& ctx) override {}
    void SaveContext(ThreadContext64& ctx) override;
    void LoadContext(const ThreadContext32& ctx) override {}
    void LoadContext(const ThreadContext64& ctx) override;

private:
    static void InterruptHook(uc_engine* uc, u32 int_no, void* user_data);

    uc_engine* uc{};
    GDBStub::BreakpointAddress last_bkpt{};
    bool last_bkpt_hit = false;
    std::size_t core_index;
};

} // namespace Core
