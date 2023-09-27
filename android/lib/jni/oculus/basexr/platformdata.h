// Copyright (c) 2017-2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "options.h"
struct PlatformData {
#ifdef XR_USE_PLATFORM_ANDROID
    void* applicationVM;
    void *applicationActivity;
#endif
};
