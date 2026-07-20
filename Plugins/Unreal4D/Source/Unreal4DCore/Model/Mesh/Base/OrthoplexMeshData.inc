#pragma once

#include "../../../Unreal4DDefines.h"

constexpr double _1D6 = 1.0 / 6.0;
constexpr double _5D6 = 5.0 / 6.0;

// This file contains pre-computed data for 4D orthoplex meshes.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define ORTHOPLEX_CELL_BOUNDARY_NORMALS TArray<FVector4> { \
	FVector4(-0.5, -0.5, -0.5, -0.5), \
	FVector4(+0.5, -0.5, -0.5, -0.5), \
	FVector4(+0.5, +0.5, -0.5, -0.5), \
	FVector4(-0.5, +0.5, -0.5, -0.5), \
	FVector4(-0.5, +0.5, +0.5, -0.5), \
	FVector4(+0.5, +0.5, +0.5, -0.5), \
	FVector4(+0.5, -0.5, +0.5, -0.5), \
	FVector4(-0.5, -0.5, +0.5, -0.5), \
	FVector4(-0.5, -0.5, +0.5, +0.5), \
	FVector4(+0.5, -0.5, +0.5, +0.5), \
	FVector4(+0.5, +0.5, +0.5, +0.5), \
	FVector4(-0.5, +0.5, +0.5, +0.5), \
	FVector4(-0.5, +0.5, -0.5, +0.5), \
	FVector4(+0.5, +0.5, -0.5, +0.5), \
	FVector4(+0.5, -0.5, -0.5, +0.5), \
	FVector4(-0.5, -0.5, -0.5, +0.5), \
}

#define REPEAT_FOUR_TIMES(m) m, m, m, m
#define ORTHOPLEX_CELL_NORMAL_INDICES TArray<int32> { \
	REPEAT_FOUR_TIMES(0), \
	REPEAT_FOUR_TIMES(1), \
	REPEAT_FOUR_TIMES(2), \
	REPEAT_FOUR_TIMES(3), \
	REPEAT_FOUR_TIMES(4), \
	REPEAT_FOUR_TIMES(5), \
	REPEAT_FOUR_TIMES(6), \
	REPEAT_FOUR_TIMES(7), \
	REPEAT_FOUR_TIMES(8), \
	REPEAT_FOUR_TIMES(9), \
	REPEAT_FOUR_TIMES(10), \
	REPEAT_FOUR_TIMES(11), \
	REPEAT_FOUR_TIMES(12), \
	REPEAT_FOUR_TIMES(13), \
	REPEAT_FOUR_TIMES(14), \
	REPEAT_FOUR_TIMES(15), \
}

#define ORTHOPLEX_CELL_VERTEX_INDICES TArray<int32> { \
	6, 0, 2, 4, 1, 2, 4, 6, 6, 1, 3, 4, 0, 3, 4, 6, /* -W and -Z */ \
	6, 0, 3, 5, 1, 3, 5, 6, 6, 1, 2, 5, 0, 2, 5, 6, /* -W and +Z */ \
	7, 0, 2, 5, 1, 2, 5, 7, 7, 1, 3, 5, 0, 3, 5, 7, /* +W and +Z */ \
	7, 0, 3, 4, 1, 3, 4, 7, 7, 1, 2, 4, 0, 2, 4, 7, /* +W and -Z */ \
}

#define ORTHOPLEX_TEXTURE_MAP_VALUES TArray<FVector> { \
	FVector(0.0, 0.5, 0.5), FVector(0.5, 0.0, 0.5), FVector(0.5, 0.5, 0.0), FVector(0.5, 0.5, 0.5), \
	FVector(0.5, 0.5, 1.0), FVector(0.5, 1.0, 0.5), FVector(1.0, 0.5, 0.5), \
	FVector(_1D6, _1D6, _1D6),  FVector(_1D6, _1D6, _5D6), FVector(_1D6, _5D6, _1D6), FVector(_1D6, _5D6, _5D6), \
	FVector(_5D6, _1D6, _1D6), FVector(_5D6, _1D6, _5D6), FVector(_5D6, _5D6, _1D6), FVector(_5D6, _5D6, _5D6), \
}

#define ORTHOPLEX_CELL_TEXTURE_MAP_INDICES TArray<int32> { \
	/* -W and -Z */ \
	7, 0, 1, 2, 6, 1, 2, 11, 13, 6, 5, 2, 0, 5, 2, 9, \
	/* -W and +Z */ \
	10, 0, 5, 4, 6, 5, 4, 14, 12, 6, 1, 4, 0, 1, 4, 8, \
	/* +W and +Z */ \
	3, 0, 1, 4, 6, 1, 4, 3, 3, 6, 5, 4, 0, 5, 4, 3, \
	/* +W and -Z */ \
	3, 0, 5, 2, 6, 5, 2, 3, 3, 6, 1, 2, 0, 1, 2, 3, \
}
/* clang-format on */
