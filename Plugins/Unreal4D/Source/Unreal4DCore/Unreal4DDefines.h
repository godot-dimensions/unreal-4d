#pragma once

// This file contains common defines. It should be included before any other file.

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnreal4DCore, Log, All);

inline constexpr double UNREAL4D_COMPARISON_EPSILON = 1e-6;
inline constexpr double UNREAL4D_TAU = 6.2831853071795864769252867666;

#if defined(__GNUC__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif
