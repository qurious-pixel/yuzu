// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <lz4hc.h>

#include "common/assert.h"
#include "common/lz4_compression.h"

namespace Common::Compression {

std::vector<u8> CompressDataLZ4(std::span<const u8> source) {
    ASSERT_MSG(source.size() <= LZ4_MAX_INPUT_SIZE, "Source size exceeds LZ4 maximum input size");

    const auto source_size_int = static_cast<int>(source.size());
    const int max_compressed_size = LZ4_compressBound(source_size_int);
    std::vector<u8> compressed(max_compressed_size);

    const int compressed_size = LZ4_compress_default(reinterpret_cast<const char*>(source.data()),
                                                     reinterpret_cast<char*>(compressed.data()),
                                                     source_size_int, max_compressed_size);

    if (compressed_size <= 0) {
        // Compression failed
        return {};
    }

    compressed.resize(compressed_size);

    return compressed;
}

std::vector<u8> CompressDataLZ4HC(std::span<const u8> source, s32 compression_level) {
    ASSERT_MSG(source.size() <= LZ4_MAX_INPUT_SIZE, "Source size exceeds LZ4 maximum input size");

    compression_level = std::clamp(compression_level, LZ4HC_CLEVEL_MIN, LZ4HC_CLEVEL_MAX);

    const auto source_size_int = static_cast<int>(source.size());
    const int max_compressed_size = LZ4_compressBound(source_size_int);
    std::vector<u8> compressed(max_compressed_size);

    const int compressed_size = LZ4_compress_HC(
        reinterpret_cast<const char*>(source.data()), reinterpret_cast<char*>(compressed.data()),
        source_size_int, max_compressed_size, compression_level);

    if (compressed_size <= 0) {
        // Compression failed
        return {};
    }

    compressed.resize(compressed_size);

    return compressed;
}

std::vector<u8> CompressDataLZ4HCMax(std::span<const u8> source) {
    return CompressDataLZ4HC(source, LZ4HC_CLEVEL_MAX);
}

std::vector<u8> DecompressDataLZ4(const std::vector<u8>& compressed,
                                  std::size_t uncompressed_size) {
    std::vector<u8> uncompressed(uncompressed_size);
    const int size_check = LZ4_decompress_safe(reinterpret_cast<const char*>(compressed.data()),
                                               reinterpret_cast<char*>(uncompressed.data()),
                                               static_cast<int>(compressed.size()),
                                               static_cast<int>(uncompressed.size()));
    if (static_cast<int>(uncompressed_size) != size_check) {
        // Decompression failed
        return {};
    }
    return uncompressed;
}

} // namespace Common::Compression
