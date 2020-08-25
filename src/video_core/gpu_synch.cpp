// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "video_core/gpu_synch.h"
#include "video_core/renderer_base.h"

namespace VideoCommon {

GPUSynch::GPUSynch(Core::System& system) : GPU{system, false} {}

GPUSynch::~GPUSynch() = default;

void GPUSynch::Start() {}

void GPUSynch::ObtainContext() {
    renderer->Context().MakeCurrent();
}

void GPUSynch::ReleaseContext() {
    renderer->Context().DoneCurrent();
}

void GPUSynch::PushGPUEntries(Tegra::CommandList&& entries) {
    dma_pusher->Push(std::move(entries));
    dma_pusher->DispatchCalls();
}

void GPUSynch::SwapBuffers(const Tegra::FramebufferConfig* framebuffer) {
    renderer->SwapBuffers(framebuffer);
}

void GPUSynch::FlushRegion(VAddr addr, u64 size) {
    renderer->Rasterizer().FlushRegion(addr, size);
}

void GPUSynch::InvalidateRegion(VAddr addr, u64 size) {
    renderer->Rasterizer().InvalidateRegion(addr, size);
}

void GPUSynch::FlushAndInvalidateRegion(VAddr addr, u64 size) {
    renderer->Rasterizer().FlushAndInvalidateRegion(addr, size);
}

} // namespace VideoCommon
