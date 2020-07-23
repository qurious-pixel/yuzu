// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/service.h"

namespace Kernel {
class HLERequestContext;
}

namespace Service::Capture {

class CAPS_C final : public ServiceFramework<CAPS_C> {
public:
    explicit CAPS_C();
    ~CAPS_C() override;
};

} // namespace Service::Capture
