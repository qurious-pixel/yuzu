// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <cstddef>
#include "common/common_types.h"
#include "video_core/engines/shader_type.h"

namespace OpenGL {

static constexpr u32 EmulationUniformBlockBinding = 0;

class Device final {
public:
    struct BaseBindings final {
        u32 uniform_buffer{};
        u32 shader_storage_buffer{};
        u32 sampler{};
        u32 image{};
    };

    explicit Device();
    explicit Device(std::nullptr_t);

    const BaseBindings& GetBaseBindings(std::size_t stage_index) const noexcept {
        return base_bindings[stage_index];
    }

    const BaseBindings& GetBaseBindings(Tegra::Engines::ShaderType shader_type) const noexcept {
        return GetBaseBindings(static_cast<std::size_t>(shader_type));
    }

    std::size_t GetUniformBufferAlignment() const {
        return uniform_buffer_alignment;
    }

    std::size_t GetShaderStorageBufferAlignment() const {
        return shader_storage_alignment;
    }

    u32 GetMaxVertexAttributes() const {
        return max_vertex_attributes;
    }

    u32 GetMaxVaryings() const {
        return max_varyings;
    }

    bool HasWarpIntrinsics() const {
        return has_warp_intrinsics;
    }

    bool HasShaderBallot() const {
        return has_shader_ballot;
    }

    bool HasVertexViewportLayer() const {
        return has_vertex_viewport_layer;
    }

    bool HasImageLoadFormatted() const {
        return has_image_load_formatted;
    }

    bool HasASTC() const {
        return has_astc;
    }

    bool HasVariableAoffi() const {
        return has_variable_aoffi;
    }

    bool HasComponentIndexingBug() const {
        return has_component_indexing_bug;
    }

    bool HasPreciseBug() const {
        return has_precise_bug;
    }

    bool HasBrokenCompute() const {
        return has_broken_compute;
    }

    bool HasFastBufferSubData() const {
        return has_fast_buffer_sub_data;
    }

    bool UseAssemblyShaders() const {
        return use_assembly_shaders;
    }

private:
    static bool TestVariableAoffi();
    static bool TestPreciseBug();

    std::array<BaseBindings, Tegra::Engines::MaxShaderTypes> base_bindings;
    std::size_t uniform_buffer_alignment{};
    std::size_t shader_storage_alignment{};
    u32 max_vertex_attributes{};
    u32 max_varyings{};
    bool has_warp_intrinsics{};
    bool has_shader_ballot{};
    bool has_vertex_viewport_layer{};
    bool has_image_load_formatted{};
    bool has_astc{};
    bool has_variable_aoffi{};
    bool has_component_indexing_bug{};
    bool has_precise_bug{};
    bool has_broken_compute{};
    bool has_fast_buffer_sub_data{};
    bool use_assembly_shaders{};
};

} // namespace OpenGL
