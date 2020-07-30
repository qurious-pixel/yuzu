// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include "common/assert.h"
#include "common/logging/log.h"
#include "core/hle/kernel/errors.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/object.h"
#include "core/hle/kernel/readable_event.h"
#include "core/hle/kernel/scheduler.h"
#include "core/hle/kernel/thread.h"

namespace Kernel {

ReadableEvent::ReadableEvent(KernelCore& kernel) : SynchronizationObject{kernel} {}
ReadableEvent::~ReadableEvent() = default;

bool ReadableEvent::ShouldWait(const Thread* thread) const {
    return !is_signaled;
}

void ReadableEvent::Acquire(Thread* thread) {
    ASSERT_MSG(IsSignaled(), "object unavailable!");
}

void ReadableEvent::Signal() {
    if (is_signaled) {
        return;
    }

    is_signaled = true;
    SynchronizationObject::Signal();
}

void ReadableEvent::Clear() {
    is_signaled = false;
}

ResultCode ReadableEvent::Reset() {
    SchedulerLock lock(kernel);
    if (!is_signaled) {
        LOG_TRACE(Kernel, "Handle is not signaled! object_id={}, object_type={}, object_name={}",
                  GetObjectId(), GetTypeName(), GetName());
        return ERR_INVALID_STATE;
    }

    Clear();

    return RESULT_SUCCESS;
}

} // namespace Kernel
