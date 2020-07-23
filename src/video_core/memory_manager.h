// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <map>
#include <optional>

#include "common/common_types.h"
#include "common/page_table.h"

namespace VideoCore {
class RasterizerInterface;
}

namespace Core {
class System;
}

namespace Tegra {

/**
 * Represents a VMA in an address space. A VMA is a contiguous region of virtual addressing space
 * with homogeneous attributes across its extents. In this particular implementation each VMA is
 * also backed by a single host memory allocation.
 */
struct VirtualMemoryArea {
    enum class Type : u8 {
        Unmapped,
        Allocated,
        Mapped,
    };

    /// Virtual base address of the region.
    GPUVAddr base{};
    /// Size of the region.
    u64 size{};
    /// Memory area mapping type.
    Type type{Type::Unmapped};
    /// CPU memory mapped address corresponding to this memory area.
    VAddr backing_addr{};
    /// Offset into the backing_memory the mapping starts from.
    std::size_t offset{};
    /// Pointer backing this VMA.
    u8* backing_memory{};

    /// Tests if this area can be merged to the right with `next`.
    bool CanBeMergedWith(const VirtualMemoryArea& next) const;
};

class MemoryManager final {
public:
    explicit MemoryManager(Core::System& system, VideoCore::RasterizerInterface& rasterizer);
    ~MemoryManager();

    GPUVAddr AllocateSpace(u64 size, u64 align);
    GPUVAddr AllocateSpace(GPUVAddr addr, u64 size, u64 align);
    GPUVAddr MapBufferEx(VAddr cpu_addr, u64 size);
    GPUVAddr MapBufferEx(VAddr cpu_addr, GPUVAddr addr, u64 size);
    GPUVAddr UnmapBuffer(GPUVAddr addr, u64 size);
    std::optional<VAddr> GpuToCpuAddress(GPUVAddr addr) const;

    template <typename T>
    T Read(GPUVAddr addr) const;

    template <typename T>
    void Write(GPUVAddr addr, T data);

    u8* GetPointer(GPUVAddr addr);
    const u8* GetPointer(GPUVAddr addr) const;

    /// Returns true if the block is continuous in host memory, false otherwise
    bool IsBlockContinuous(GPUVAddr start, std::size_t size) const;

    /**
     * ReadBlock and WriteBlock are full read and write operations over virtual
     * GPU Memory. It's important to use these when GPU memory may not be continuous
     * in the Host Memory counterpart. Note: This functions cause Host GPU Memory
     * Flushes and Invalidations, respectively to each operation.
     */
    void ReadBlock(GPUVAddr gpu_src_addr, void* dest_buffer, std::size_t size) const;
    void WriteBlock(GPUVAddr gpu_dest_addr, const void* src_buffer, std::size_t size);
    void CopyBlock(GPUVAddr gpu_dest_addr, GPUVAddr gpu_src_addr, std::size_t size);

    /**
     * ReadBlockUnsafe and WriteBlockUnsafe are special versions of ReadBlock and
     * WriteBlock respectively. In this versions, no flushing or invalidation is actually
     * done and their performance is similar to a memcpy. This functions can be used
     * on either of this 2 scenarios instead of their safe counterpart:
     * - Memory which is sure to never be represented in the Host GPU.
     * - Memory Managed by a Cache Manager. Example: Texture Flushing should use
     * WriteBlockUnsafe instead of WriteBlock since it shouldn't invalidate the texture
     * being flushed.
     */
    void ReadBlockUnsafe(GPUVAddr gpu_src_addr, void* dest_buffer, std::size_t size) const;
    void WriteBlockUnsafe(GPUVAddr gpu_dest_addr, const void* src_buffer, std::size_t size);
    void CopyBlockUnsafe(GPUVAddr gpu_dest_addr, GPUVAddr gpu_src_addr, std::size_t size);

    /**
     * IsGranularRange checks if a gpu region can be simply read with a pointer
     */
    bool IsGranularRange(GPUVAddr gpu_addr, std::size_t size);

private:
    using VMAMap = std::map<GPUVAddr, VirtualMemoryArea>;
    using VMAHandle = VMAMap::const_iterator;
    using VMAIter = VMAMap::iterator;

    bool IsAddressValid(GPUVAddr addr) const;
    void MapPages(GPUVAddr base, u64 size, u8* memory, Common::PageType type,
                  VAddr backing_addr = 0);
    void MapMemoryRegion(GPUVAddr base, u64 size, u8* target, VAddr backing_addr);
    void UnmapRegion(GPUVAddr base, u64 size);

    /// Finds the VMA in which the given address is included in, or `vma_map.end()`.
    VMAHandle FindVMA(GPUVAddr target) const;

    VMAHandle AllocateMemory(GPUVAddr target, std::size_t offset, u64 size);

    /**
     * Maps an unmanaged host memory pointer at a given address.
     *
     * @param target       The guest address to start the mapping at.
     * @param memory       The memory to be mapped.
     * @param size         Size of the mapping in bytes.
     * @param backing_addr The base address of the range to back this mapping.
     */
    VMAHandle MapBackingMemory(GPUVAddr target, u8* memory, u64 size, VAddr backing_addr);

    /// Unmaps a range of addresses, splitting VMAs as necessary.
    void UnmapRange(GPUVAddr target, u64 size);

    /// Converts a VMAHandle to a mutable VMAIter.
    VMAIter StripIterConstness(const VMAHandle& iter);

    /// Marks as the specified VMA as allocated.
    VMAIter Allocate(VMAIter vma);

    /**
     * Carves a VMA of a specific size at the specified address by splitting Free VMAs while doing
     * the appropriate error checking.
     */
    VMAIter CarveVMA(GPUVAddr base, u64 size);

    /**
     * Splits the edges of the given range of non-Free VMAs so that there is a VMA split at each
     * end of the range.
     */
    VMAIter CarveVMARange(GPUVAddr base, u64 size);

    /**
     * Splits a VMA in two, at the specified offset.
     * @returns the right side of the split, with the original iterator becoming the left side.
     */
    VMAIter SplitVMA(VMAIter vma, u64 offset_in_vma);

    /**
     * Checks for and merges the specified VMA with adjacent ones if possible.
     * @returns the merged VMA or the original if no merging was possible.
     */
    VMAIter MergeAdjacent(VMAIter vma);

    /// Updates the pages corresponding to this VMA so they match the VMA's attributes.
    void UpdatePageTableForVMA(const VirtualMemoryArea& vma);

    /// Finds a free (unmapped region) of the specified size starting at the specified address.
    GPUVAddr FindFreeRegion(GPUVAddr region_start, u64 size) const;

private:
    static constexpr u64 page_bits{16};
    static constexpr u64 page_size{1 << page_bits};
    static constexpr u64 page_mask{page_size - 1};

    /// Address space in bits, according to Tegra X1 TRM
    static constexpr u32 address_space_width{40};
    /// Start address for mapping, this is fairly arbitrary but must be non-zero.
    static constexpr GPUVAddr address_space_base{0x100000};
    /// End of address space, based on address space in bits.
    static constexpr GPUVAddr address_space_end{1ULL << address_space_width};

    Common::PageTable page_table;
    VMAMap vma_map;
    VideoCore::RasterizerInterface& rasterizer;

    Core::System& system;
};

} // namespace Tegra
