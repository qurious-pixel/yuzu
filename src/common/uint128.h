// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <utility>
#include "common/common_types.h"

namespace Common {

// This function multiplies 2 u64 values and divides it by a u64 value.
[[nodiscard]] u64 MultiplyAndDivide64(u64 a, u64 b, u64 d);

// This function multiplies 2 u64 values and produces a u128 value;
[[nodiscard]] u128 Multiply64Into128(u64 a, u64 b);

// This function divides a u128 by a u32 value and produces two u64 values:
// the result of division and the remainder
[[nodiscard]] std::pair<u64, u64> Divide128On32(u128 dividend, u32 divisor);

} // namespace Common
