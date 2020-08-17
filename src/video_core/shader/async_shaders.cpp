// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include "video_core/engines/maxwell_3d.h"
#include "video_core/renderer_base.h"
#include "video_core/renderer_opengl/gl_shader_cache.h"
#include "video_core/shader/async_shaders.h"

namespace VideoCommon::Shader {

AsyncShaders::AsyncShaders(Core::Frontend::EmuWindow& emu_window) : emu_window(emu_window) {}

AsyncShaders::~AsyncShaders() {
    KillWorkers();
}

void AsyncShaders::AllocateWorkers() {
    // Max worker threads we should allow
    constexpr u32 MAX_THREADS = 4;
    // Deduce how many threads we can use
    const u32 threads_used = std::thread::hardware_concurrency() / 4;
    // Always allow at least 1 thread regardless of our settings
    const auto max_worker_count = std::max(1U, threads_used);
    // Don't use more than MAX_THREADS
    const auto num_workers = std::min(max_worker_count, MAX_THREADS);

    // If we already have workers queued, ignore
    if (num_workers == worker_threads.size()) {
        return;
    }

    // If workers already exist, clear them
    if (!worker_threads.empty()) {
        FreeWorkers();
    }

    // Create workers
    for (std::size_t i = 0; i < num_workers; i++) {
        context_list.push_back(emu_window.CreateSharedContext());
        worker_threads.push_back(
            std::thread(&AsyncShaders::ShaderCompilerThread, this, context_list[i].get()));
    }
}

void AsyncShaders::FreeWorkers() {
    // Mark all threads to quit
    is_thread_exiting.store(true);
    cv.notify_all();
    for (auto& thread : worker_threads) {
        thread.join();
    }
    // Clear our shared contexts
    context_list.clear();

    // Clear our worker threads
    worker_threads.clear();
}

void AsyncShaders::KillWorkers() {
    is_thread_exiting.store(true);
    for (auto& thread : worker_threads) {
        thread.detach();
    }
    // Clear our shared contexts
    context_list.clear();

    // Clear our worker threads
    worker_threads.clear();
}

bool AsyncShaders::HasWorkQueued() {
    return !pending_queue.empty();
}

bool AsyncShaders::HasCompletedWork() {
    std::shared_lock lock{completed_mutex};
    return !finished_work.empty();
}

bool AsyncShaders::IsShaderAsync(const Tegra::GPU& gpu) const {
    const auto& regs = gpu.Maxwell3D().regs;

    // If something is using depth, we can assume that games are not rendering anything which will
    // be used one time.
    if (regs.zeta_enable) {
        return true;
    }

    // If games are using a small index count, we can assume these are full screen quads. Usually
    // these shaders are only used once for building textures so we can assume they can't be built
    // async
    if (regs.index_array.count <= 6 || regs.vertex_buffer.count <= 6) {
        return false;
    }

    return true;
}

std::vector<AsyncShaders::Result> AsyncShaders::GetCompletedWork() {
    std::vector<AsyncShaders::Result> results;
    {
        std::unique_lock lock{completed_mutex};
        results.assign(std::make_move_iterator(finished_work.begin()),
                       std::make_move_iterator(finished_work.end()));
        finished_work.clear();
    }
    return results;
}

void AsyncShaders::QueueOpenGLShader(const OpenGL::Device& device,
                                     Tegra::Engines::ShaderType shader_type, u64 uid,
                                     std::vector<u64> code, std::vector<u64> code_b,
                                     u32 main_offset,
                                     VideoCommon::Shader::CompilerSettings compiler_settings,
                                     const VideoCommon::Shader::Registry& registry,
                                     VAddr cpu_addr) {
    WorkerParams params{
        .backend = device.UseAssemblyShaders() ? Backend::GLASM : Backend::OpenGL,
        .device = &device,
        .shader_type = shader_type,
        .uid = uid,
        .code = std::move(code),
        .code_b = std::move(code_b),
        .main_offset = main_offset,
        .compiler_settings = compiler_settings,
        .registry = registry,
        .cpu_address = cpu_addr,
    };
    std::unique_lock lock(queue_mutex);
    pending_queue.push(std::move(params));
    cv.notify_one();
}

void AsyncShaders::QueueVulkanShader(Vulkan::VKPipelineCache* pp_cache,
                                     const Vulkan::VKDevice& device, Vulkan::VKScheduler& scheduler,
                                     Vulkan::VKDescriptorPool& descriptor_pool,
                                     Vulkan::VKUpdateDescriptorQueue& update_descriptor_queue,
                                     Vulkan::VKRenderPassCache& renderpass_cache,
                                     std::vector<VkDescriptorSetLayoutBinding> bindings,
                                     Vulkan::SPIRVProgram program,
                                     Vulkan::GraphicsPipelineCacheKey key) {
    WorkerParams params{
        .backend = Backend::Vulkan,
        .pp_cache = pp_cache,
        .vk_device = &device,
        .scheduler = &scheduler,
        .descriptor_pool = &descriptor_pool,
        .update_descriptor_queue = &update_descriptor_queue,
        .renderpass_cache = &renderpass_cache,
        .bindings = bindings,
        .program = program,
        .key = key,
    };

    std::unique_lock lock(queue_mutex);
    pending_queue.push(std::move(params));
    cv.notify_one();
}

void AsyncShaders::ShaderCompilerThread(Core::Frontend::GraphicsContext* context) {
    while (!is_thread_exiting.load(std::memory_order_relaxed)) {
        std::unique_lock lock{queue_mutex};
        cv.wait(lock, [this] { return HasWorkQueued() || is_thread_exiting; });
        if (is_thread_exiting) {
            return;
        }

        // Partial lock to allow all threads to read at the same time
        if (!HasWorkQueued()) {
            continue;
        }
        // Another thread beat us, just unlock and wait for the next load
        if (pending_queue.empty()) {
            continue;
        }

        // Pull work from queue
        WorkerParams work = std::move(pending_queue.front());
        pending_queue.pop();
        lock.unlock();

        if (work.backend == Backend::OpenGL || work.backend == Backend::GLASM) {
            const ShaderIR ir(work.code, work.main_offset, work.compiler_settings, *work.registry);
            const auto scope = context->Acquire();
            auto program =
                OpenGL::BuildShader(*work.device, work.shader_type, work.uid, ir, *work.registry);
            Result result{};
            result.backend = work.backend;
            result.cpu_address = work.cpu_address;
            result.uid = work.uid;
            result.code = std::move(work.code);
            result.code_b = std::move(work.code_b);
            result.shader_type = work.shader_type;

            if (work.backend == Backend::OpenGL) {
                result.program.opengl = std::move(program->source_program);
            } else if (work.backend == Backend::GLASM) {
                result.program.glasm = std::move(program->assembly_program);
            }

            {
                std::unique_lock complete_lock(completed_mutex);
                finished_work.push_back(std::move(result));
            }
        } else if (work.backend == Backend::Vulkan) {
            auto pipeline = std::make_unique<Vulkan::VKGraphicsPipeline>(
                *work.vk_device, *work.scheduler, *work.descriptor_pool,
                *work.update_descriptor_queue, *work.renderpass_cache, work.key, work.bindings,
                work.program);

            work.pp_cache->EmplacePipeline(std::move(pipeline));
        }
    }
}

} // namespace VideoCommon::Shader
