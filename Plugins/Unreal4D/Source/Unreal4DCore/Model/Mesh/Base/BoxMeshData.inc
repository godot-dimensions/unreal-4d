#pragma once

#include "../../../Unreal4DDefines.h"

// This file contains pre-computed data for 4D box meshes.
// This code is much harder to read when clang-format messes with the
// manual alignment, so disable clang-format just for this file.
/* clang-format off */
#define REPEAT_FIVE_TIMES(m) m, m, m, m, m
#define BOX_40_CELL_BOUNDARY_NORMALS TArray<FVector4> { \
	REPEAT_FIVE_TIMES(FVector4(-1.0, 0.0, 0.0, 0.0)), /* -X */ \
	REPEAT_FIVE_TIMES(FVector4(+1.0, 0.0, 0.0, 0.0)), /* +X */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, -1.0, 0.0, 0.0)), /* -Y */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, +1.0, 0.0, 0.0)), /* +Y */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, 0.0, -1.0, 0.0)), /* -Z */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, 0.0, +1.0, 0.0)), /* +Z */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, 0.0, 0.0, -1.0)), /* -W */ \
	REPEAT_FIVE_TIMES(FVector4(0.0, 0.0, 0.0, +1.0)), /* +W */ \
}

#define REPEAT_SIX_TIMES(m) m, m, m, m, m, m
#define BOX_48_CELL_BOUNDARY_NORMALS TArray<FVector4> { \
	REPEAT_SIX_TIMES(FVector4(-1.0, 0.0, 0.0, 0.0)), /* -X */ \
	REPEAT_SIX_TIMES(FVector4(+1.0, 0.0, 0.0, 0.0)), /* +X */ \
	REPEAT_SIX_TIMES(FVector4(0.0, -1.0, 0.0, 0.0)), /* -Y */ \
	REPEAT_SIX_TIMES(FVector4(0.0, +1.0, 0.0, 0.0)), /* +Y */ \
	REPEAT_SIX_TIMES(FVector4(0.0, 0.0, -1.0, 0.0)), /* -Z */ \
	REPEAT_SIX_TIMES(FVector4(0.0, 0.0, +1.0, 0.0)), /* +Z */ \
	REPEAT_SIX_TIMES(FVector4(0.0, 0.0, 0.0, -1.0)), /* -W */ \
	REPEAT_SIX_TIMES(FVector4(0.0, 0.0, 0.0, +1.0)), /* +W */ \
}

#define REPEAT_SIX_TIMES(m) m, m, m, m, m, m
#define BOX_CELL_NORMAL_VALUES TArray<FVector4> { \
	FVector4(-1.0, 0.0, 0.0, 0.0), /* -X */ \
	FVector4(+1.0, 0.0, 0.0, 0.0), /* +X */ \
	FVector4(0.0, -1.0, 0.0, 0.0), /* -Y */ \
	FVector4(0.0, +1.0, 0.0, 0.0), /* +Y */ \
	FVector4(0.0, 0.0, -1.0, 0.0), /* -Z */ \
	FVector4(0.0, 0.0, +1.0, 0.0), /* +Z */ \
	FVector4(0.0, 0.0, 0.0, -1.0), /* -W */ \
	FVector4(0.0, 0.0, 0.0, +1.0), /* +W */ \
}

#define REPEAT_TWENTY_TIMES(m) m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m
#define BOX_40_CELL_NORMAL_INDICES TArray<int32> { \
	REPEAT_TWENTY_TIMES(0), /* -X */ \
	REPEAT_TWENTY_TIMES(1), /* +X */ \
	REPEAT_TWENTY_TIMES(2), /* -Y */ \
	REPEAT_TWENTY_TIMES(3), /* +Y */ \
	REPEAT_TWENTY_TIMES(4), /* -Z */ \
	REPEAT_TWENTY_TIMES(5), /* +Z */ \
	REPEAT_TWENTY_TIMES(6), /* -W */ \
	REPEAT_TWENTY_TIMES(7), /* +W */ \
}

#define REPEAT_TWENTY_FOUR_TIMES(m) m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m, m
#define BOX_48_CELL_NORMAL_INDICES TArray<int32> { \
	REPEAT_TWENTY_FOUR_TIMES(0), /* -X */ \
	REPEAT_TWENTY_FOUR_TIMES(1), /* +X */ \
	REPEAT_TWENTY_FOUR_TIMES(2), /* -Y */ \
	REPEAT_TWENTY_FOUR_TIMES(3), /* +Y */ \
	REPEAT_TWENTY_FOUR_TIMES(4), /* -Z */ \
	REPEAT_TWENTY_FOUR_TIMES(5), /* +Z */ \
	REPEAT_TWENTY_FOUR_TIMES(6), /* -W */ \
	REPEAT_TWENTY_FOUR_TIMES(7), /* +W */ \
}

#define BOX_40_CELL_VERTEX_INDICES TArray<int32> { \
	 8,  4, 14,  2,  8, 10,  2, 14,  8,  0,  4,  2,  8, 12, 14,  4,  6,  4,  2, 14, /* -X */ \
	11, 13,  1,  7, 11,  3,  7,  1, 11, 15, 13,  7, 11,  9,  1, 13,  5, 13,  7,  1, /* +X */ \
	 4, 13,  1,  8,  4,  0,  8,  1,  4, 12, 13,  8,  4,  5,  1, 13,  9, 13,  8,  1, /* -Y */ \
	 14, 7, 11,  2, 14, 10,  2, 11, 14,  6,  7,  2, 14, 15, 11,  7,  3,  7,  2, 11, /* +Y */ \
	 1,  8, 11,  2,  1,  3,  2, 11,  1,  0,  8,  2,  1,  9, 11,  8, 10,  8,  2, 11, /* -Z */ \
	 13, 4,  7, 14, 13, 15, 14,  7, 13, 12,  4, 14, 13,  5,  7,  4,  6,  4, 14,  7, /* +Z */ \
	 1,  7,  4,  2,  1,  0,  2,  4,  1,  3,  7,  2,  1,  5,  4,  7,  6,  7,  2,  4, /* -W */ \
	 8, 14, 13, 11,  8,  9, 11, 13,  8, 10, 14, 11,  8, 12, 13, 14, 15, 14, 11, 13, /* +W */ \
}

#define BOX_48_CELL_VERTEX_INDICES TArray<int32> { \
	 8,  0,  4,  6,  8,  4, 12,  6,  8, 12, 14,  6,  8, 14, 10,  6,  8, 10,  2,  6,  8,  2,  0,  6, /* -X */ \
	 9, 11, 15,  7,  9, 15, 13,  7,  9, 13,  5,  7,  9,  5,  1,  7,  9,  1,  3,  7,  9,  3, 11,  7, /* +X */ \
	 5, 13,  9,  8,  5,  9,  1,  8,  5,  1,  0,  8,  5,  0,  4,  8,  5,  4, 12,  8,  5, 12, 13,  8, /* -Y */ \
	10,  2,  6,  7, 10,  6, 14,  7, 10, 14, 15,  7, 10, 15, 11,  7, 10, 11,  3,  7, 10,  3,  2,  7, /* +Y */ \
	 8,  0,  2,  3,  8,  2, 10,  3,  8, 10, 11,  3,  8, 11,  9,  3,  8,  9,  1,  3,  8,  1,  0,  3, /* -Z */ \
	12, 14,  6,  7, 12,  6,  4,  7, 12,  4,  5,  7, 12,  5, 13,  7, 12, 13, 15,  7, 12, 15, 14,  7, /* +Z */ \
	 0,  1,  5,  7,  0,  3,  1,  7,  0,  2,  3,  7,  0,  6,  2,  7,  0,  4,  6,  7,  0,  5,  4,  7, /* -W */ \
	 8, 10, 14, 15,  8, 14, 12, 15,  8, 12, 13, 15,  8, 13,  9, 15,  8,  9, 11, 15,  8, 11, 10, 15, /* +W */ \
}

// C++ hex floating point literals suck, but this means that each side is 5/16ths of the texture in size.
#define BOX_TEXTURE_MAP_VALUES_CROSS_ISLAND TArray<FVector> { \
	FVector(0x0.0p0, 0x0.5p0, 0x0.5p0), FVector(0x0.0p0, 0x0.5p0, 0x0.Ap0), FVector(0x0.0p0, 0x0.Ap0, 0x0.5p0), FVector(0x0.0p0, 0x0.Ap0, 0x0.Ap0), \
	FVector(0x0.5p0, 0x0.0p0, 0x0.5p0), FVector(0x0.5p0, 0x0.0p0, 0x0.Ap0), FVector(0x0.5p0, 0x0.5p0, 0x0.0p0), FVector(0x0.5p0, 0x0.5p0, 0x0.5p0), \
	FVector(0x0.5p0, 0x0.5p0, 0x0.Ap0), FVector(0x0.5p0, 0x0.5p0, 0x0.Fp0), FVector(0x0.5p0, 0x0.Ap0, 0x0.0p0), FVector(0x0.5p0, 0x0.Ap0, 0x0.5p0), \
	FVector(0x0.5p0, 0x0.Ap0, 0x0.Ap0), FVector(0x0.5p0, 0x0.Ap0, 0x0.Fp0), FVector(0x0.5p0, 0x0.Fp0, 0x0.5p0), FVector(0x0.5p0, 0x0.Fp0, 0x0.Ap0), \
	FVector(0x0.Ap0, 0x0.0p0, 0x0.5p0), FVector(0x0.Ap0, 0x0.0p0, 0x0.Ap0), FVector(0x0.Ap0, 0x0.5p0, 0x0.0p0), FVector(0x0.Ap0, 0x0.5p0, 0x0.5p0), \
	FVector(0x0.Ap0, 0x0.5p0, 0x0.Ap0), FVector(0x0.Ap0, 0x0.5p0, 0x0.Fp0), FVector(0x0.Ap0, 0x0.Ap0, 0x0.0p0), FVector(0x0.Ap0, 0x0.Ap0, 0x0.5p0), \
	FVector(0x0.Ap0, 0x0.Ap0, 0x0.Ap0), FVector(0x0.Ap0, 0x0.Ap0, 0x0.Fp0), FVector(0x0.Ap0, 0x0.Fp0, 0x0.5p0), FVector(0x0.Ap0, 0x0.Fp0, 0x0.Ap0), \
	FVector(0x0.Bp0, 0x0.Bp0, 0x0.Bp0), FVector(0x0.Bp0, 0x0.Bp0, 0x1.0p0), FVector(0x0.Bp0, 0x1.0p0, 0x0.Bp0), FVector(0x0.Bp0, 0x1.0p0, 0x1.0p0), \
	FVector(0x0.Fp0, 0x0.5p0, 0x0.5p0), FVector(0x0.Fp0, 0x0.5p0, 0x0.Ap0), FVector(0x0.Fp0, 0x0.Ap0, 0x0.5p0), FVector(0x0.Fp0, 0x0.Ap0, 0x0.Ap0), \
	FVector(0x1.0p0, 0x0.Bp0, 0x0.Bp0), FVector(0x1.0p0, 0x0.Bp0, 0x1.0p0), FVector(0x1.0p0, 0x1.0p0, 0x0.Bp0), FVector(0x1.0p0, 0x1.0p0, 0x1.0p0), \
}

#define BOX_TEXTURE_MAP_VALUES_FILL_EACH_SIDE TArray<FVector> { \
	FVector(0.0, 0.0, 0.0), FVector(0.0, 0.0, 1.0), FVector(0.0, 1.0, 0.0), FVector(0.0, 1.0, 1.0), \
	FVector(1.0, 0.0, 0.0), FVector(1.0, 0.0, 1.0), FVector(1.0, 1.0, 0.0), FVector(1.0, 1.0, 1.0), \
}

#define BOX_TEXTURE_MAP_VALUES_COMPACT_2X2X2_GRID TArray<FVector> { \
	FVector(0.0, 0.0, 0.0), FVector(0.0, 0.0, 0.5), FVector(0.0, 0.0, 1.0), FVector(0.0, 0.5, 0.0), \
	FVector(0.0, 0.5, 0.5), FVector(0.0, 0.5, 1.0), FVector(0.0, 1.0, 0.0), FVector(0.0, 1.0, 0.5), \
	FVector(0.0, 1.0, 1.0), FVector(0.5, 0.0, 0.0), FVector(0.5, 0.0, 0.5), FVector(0.5, 0.0, 1.0), \
	FVector(0.5, 0.5, 0.0), FVector(0.5, 0.5, 0.5), FVector(0.5, 0.5, 1.0), FVector(0.5, 1.0, 0.0), \
	FVector(0.5, 1.0, 0.5), FVector(0.5, 1.0, 1.0), FVector(1.0, 0.0, 0.0), FVector(1.0, 0.0, 0.5), \
	FVector(1.0, 0.0, 1.0), FVector(1.0, 0.5, 0.0), FVector(1.0, 0.5, 0.5), FVector(1.0, 0.5, 1.0), \
	FVector(1.0, 1.0, 0.0), FVector(1.0, 1.0, 0.5), FVector(1.0, 1.0, 1.0), \
}

#define _0_3RDS 0.0
#define _1_3RDS (1.0 / 3.0)
#define _2_3RDS (2.0 / 3.0)
#define _3_3RDS 1.0

#define BOX_TEXTURE_MAP_VALUES_COMPACT_LONG_CROSS TArray<FVector> { \
	FVector(0x0.0p0, _1_3RDS, _1_3RDS), FVector(0x0.0p0, _1_3RDS, _2_3RDS), FVector(0x0.0p0, _2_3RDS, _1_3RDS), FVector(0x0.0p0, _2_3RDS, _2_3RDS), \
	FVector(0x0.4p0, _0_3RDS, _1_3RDS), FVector(0x0.4p0, _0_3RDS, _2_3RDS), FVector(0x0.4p0, _1_3RDS, _0_3RDS), FVector(0x0.4p0, _1_3RDS, _1_3RDS), \
	FVector(0x0.4p0, _1_3RDS, _2_3RDS), FVector(0x0.4p0, _1_3RDS, _3_3RDS), FVector(0x0.4p0, _2_3RDS, _0_3RDS), FVector(0x0.4p0, _2_3RDS, _1_3RDS), \
	FVector(0x0.4p0, _2_3RDS, _2_3RDS), FVector(0x0.4p0, _2_3RDS, _3_3RDS), FVector(0x0.4p0, _3_3RDS, _1_3RDS), FVector(0x0.4p0, _3_3RDS, _2_3RDS), \
	FVector(0x0.8p0, _0_3RDS, _1_3RDS), FVector(0x0.8p0, _0_3RDS, _2_3RDS), FVector(0x0.8p0, _1_3RDS, _0_3RDS), FVector(0x0.8p0, _1_3RDS, _1_3RDS), \
	FVector(0x0.8p0, _1_3RDS, _2_3RDS), FVector(0x0.8p0, _1_3RDS, _3_3RDS), FVector(0x0.8p0, _2_3RDS, _0_3RDS), FVector(0x0.8p0, _2_3RDS, _1_3RDS), \
	FVector(0x0.8p0, _2_3RDS, _2_3RDS), FVector(0x0.8p0, _2_3RDS, _3_3RDS), FVector(0x0.8p0, _3_3RDS, _1_3RDS), FVector(0x0.8p0, _3_3RDS, _2_3RDS), \
	FVector(0x0.Cp0, _1_3RDS, _1_3RDS), FVector(0x0.Cp0, _1_3RDS, _2_3RDS), FVector(0x0.Cp0, _2_3RDS, _1_3RDS), FVector(0x0.Cp0, _2_3RDS, _2_3RDS), \
	FVector(0x1.0p0, _1_3RDS, _1_3RDS), FVector(0x1.0p0, _1_3RDS, _2_3RDS), FVector(0x1.0p0, _2_3RDS, _1_3RDS), FVector(0x1.0p0, _2_3RDS, _2_3RDS), \
}

#define BOX_40_CELL_TEXTURE_MAP_INDICES_CROSS_ISLAND TArray<int32> { \
	/* -X */ \
	7, 1, 12, 2, 7, 11, 2, 12, 7, 0, 1, 2, 7, 8, 12, 1, 3, 1, 2, 12, \
	/* +X */ \
	23, 20, 32, 35, 23, 34, 35, 32, 23, 24, 20, 35, 23, 19, 32, 20, 33, 20, 35, 32, \
	/* -Y */ \
	5, 20, 16, 7, 5, 4, 7, 16, 5, 8, 20, 7, 5, 17, 16, 20, 19, 20, 7, 16, \
	/* +Y */ \
	12, 27, 23, 14, 12, 11, 14, 23, 12, 15, 27, 14, 12, 24, 23, 27, 26, 27, 14, 23, \
	/* -Z */ \
	18, 7, 23, 10, 18, 22, 10, 23, 18, 6, 7, 10, 18, 19, 23, 7, 11, 7, 10, 23, \
	/* +Z */ \
	20, 9, 25, 12, 20, 24, 12, 25, 20, 8, 9, 12, 20, 21, 25, 9, 13, 9, 12, 25, \
	/* -W */ \
	28, 31, 37, 38, 28, 36, 38, 37, 28, 30, 31, 38, 28, 29, 37, 31, 39, 31, 38, 37, \
	/* +W */ \
	7, 12, 20, 23, 7, 19, 23, 20, 7, 11, 12, 23, 7, 8, 20, 12, 24, 12, 23, 20, \
}

#define BOX_40_CELL_TEXTURE_MAP_INDICES_FILL_EACH_SIDE TArray<int32> { \
	/* -X */ \
	4, 1, 7, 2, 4, 6, 2, 7, 4, 0, 1, 2, 4, 5, 7, 1, 3, 1, 2, 7, \
	/* +X */ \
	2, 1, 4, 7, 2, 6, 7, 4, 2, 3, 1, 7, 2, 0, 4, 1, 5, 1, 7, 4, \
	/* -Y */ \
	1, 7, 4, 2, 1, 0, 2, 4, 1, 3, 7, 2, 1, 5, 4, 7, 6, 7, 2, 4, \
	/* +Y */ \
	1, 7, 4, 2, 1, 0, 2, 4, 1, 3, 7, 2, 1, 5, 4, 7, 6, 7, 2, 4, \
	/* -Z */ \
	4, 1, 7, 2, 4, 6, 2, 7, 4, 0, 1, 2, 4, 5, 7, 1, 3, 1, 2, 7, \
	/* +Z */ \
	4, 1, 7, 2, 4, 6, 2, 7, 4, 0, 1, 2, 4, 5, 7, 1, 3, 1, 2, 7, \
	/* -W */ \
	0, 3, 5, 6, 0, 4, 6, 5, 0, 2, 3, 6, 0, 1, 5, 3, 7, 3, 6, 5, \
	/* +W */ \
	0, 3, 5, 6, 0, 4, 6, 5, 0, 2, 3, 6, 0, 1, 5, 3, 7, 3, 6, 5 \
}

#define BOX_40_CELL_TEXTURE_MAP_INDICES_COMPACT_2X2X2_GRID TArray<int32> { \
	/* -X */ \
	8, 16, 4, 14, 8, 5, 14, 4, 8, 17, 16, 14, 8, 7, 4, 16, 13, 16, 14, 4, \
	/* +X */ \
	12, 10, 18, 22, 12, 21, 22, 18, 12, 13, 10, 22, 12, 9, 18, 10, 19, 10, 22, 18, \
	/* -Y */ \
	22, 10, 14, 20, 22, 23, 20, 14, 22, 19, 10, 20, 22, 13, 14, 10, 11, 10, 20, 14, \
	/* +Y */ \
	4, 16, 12, 6, 4, 3, 6, 12, 4, 7, 16, 6, 4, 13, 12, 16, 15, 16, 6, 12, \
	/* -Z */ \
	16, 24, 12, 22, 16, 13, 22, 12, 16, 25, 24, 22, 16, 15, 12, 24, 21, 24, 22, 12, \
	/* +Z */ \
	10, 2, 14, 4, 10, 13, 4, 14, 10, 1, 2, 4, 10, 11, 14, 2, 5, 2, 4, 14, \
	/* -W */ \
	26, 22, 16, 14, 26, 17, 14, 16, 26, 23, 22, 14, 26, 25, 16, 22, 13, 22, 14, 16, \
	/* +W */ \
	0, 4, 10, 12, 0, 9, 12, 10, 0, 3, 4, 12, 0, 1, 10, 4, 13, 4, 12, 10 \
}

#define BOX_40_CELL_TEXTURE_MAP_INDICES_LONG_CROSS TArray<int32> { \
	/* -X */ \
	7, 1, 12, 2, 7, 11, 2, 12, 7, 0, 1, 2, 7, 8, 12, 1, 3, 1, 2, 12, \
	/* +X */ \
	23, 20, 28, 31, 23, 30, 31, 28, 23, 24, 20, 31, 23, 19, 28, 20, 29, 20, 31, 28, \
	/* -Y */ \
	5, 20, 16, 7, 5, 4, 7, 16, 5, 8, 20, 7, 5, 17, 16, 20, 19, 20, 7, 16, \
	/* +Y */ \
	12, 27, 23, 14, 12, 11, 14, 23, 12, 15, 27, 14, 12, 24, 23, 27, 26, 27, 14, 23, \
	/* -Z */ \
	18, 7, 23, 10, 18, 22, 10, 23, 18, 6, 7, 10, 18, 19, 23, 7, 11, 7, 10, 23, \
	/* +Z */ \
	20, 9, 25, 12, 20, 24, 12, 25, 20, 8, 9, 12, 20, 21, 25, 9, 13, 9, 12, 25, \
	/* -W */ \
	28, 31, 33, 34, 28, 32, 34, 33, 28, 30, 31, 34, 28, 29, 33, 31, 35, 31, 34, 33, \
	/* +W */ \
	7, 12, 20, 23, 7, 19, 23, 20, 7, 11, 12, 23, 7, 8, 20, 12, 24, 12, 23, 20, \
}

#define BOX_48_CELL_TEXTURE_MAP_INDICES_CROSS_ISLAND TArray<int32> { \
	/* -X */ \
	7, 0, 1, 3, 7, 1, 8, 3, 7, 8, 12, 3, 7, 12, 11, 3, 7, 11, 2, 3, 7, 2, 0, 3, \
	/* +X */ \
	19, 23, 24, 35, 19, 24, 20, 35, 19, 20, 33, 35, 19, 33, 32, 35, 19, 32, 34, 35, 19, 34, 23, 35, \
	/* -Y */ \
	17, 20, 19, 7, 17, 19, 16, 7, 17, 16, 4, 7, 17, 4, 5, 7, 17, 5, 8, 7, 17, 8, 20, 7, \
	/* +Y */ \
	11, 14, 15, 27, 11, 15, 12, 27, 11, 12, 24, 27, 11, 24, 23, 27, 11, 23, 26, 27, 11, 26, 14, 27, \
	/* -Z */ \
	7, 6, 10, 22, 7, 10, 11, 22, 7, 11, 23, 22, 7, 23, 19, 22, 7, 19, 18, 22, 7, 18, 6, 22, \
	/* +Z */ \
	8, 12, 13, 25, 8, 13, 9, 25, 8, 9, 21, 25, 8, 21, 20, 25, 8, 20, 24, 25, 8, 24, 12, 25, \
	/* -W */ \
	39, 31, 30, 28, 39, 29, 31, 28, 39, 37, 29, 28, 39, 36, 37, 28, 39, 38, 36, 28, 39, 30, 38, 28, \
	/* +W */ \
	7, 11, 12, 24, 7, 12, 8, 24, 7, 8, 20, 24, 7, 20, 19, 24, 7, 19, 23, 24, 7, 23, 11, 24, \
}

#define BOX_48_CELL_TEXTURE_MAP_INDICES_FILL_EACH_SIDE TArray<int32> { \
	/* -X */ \
	4, 0, 1, 3, 4, 1, 5, 3, 4, 5, 7, 3, 4, 7, 6, 3, 4, 6, 2, 3, 4, 2, 0, 3, \
	/* +X */ \
	0, 2, 3, 7, 0, 3, 1, 7, 0, 1, 5, 7, 0, 5, 4, 7, 0, 4, 6, 7, 0, 6, 2, 7, \
	/* -Y */ \
	5, 7, 6, 2, 5, 6, 4, 2, 5, 4, 0, 2, 5, 0, 1, 2, 5, 1, 3, 2, 5, 3, 7, 2, \
	/* +Y */ \
	0, 2, 3, 7, 0, 3, 1, 7, 0, 1, 5, 7, 0, 5, 4, 7, 0, 4, 6, 7, 0, 6, 2, 7, \
	/* -Z */ \
	1, 0, 2, 6, 1, 2, 3, 6, 1, 3, 7, 6, 1, 7, 5, 6, 1, 5, 4, 6, 1, 4, 0, 6, \
	/* +Z */ \
	0, 2, 3, 7, 0, 3, 1, 7, 0, 1, 5, 7, 0, 5, 4, 7, 0, 4, 6, 7, 0, 6, 2, 7, \
	/* -W */ \
	7, 3, 2, 0, 7, 1, 3, 0, 7, 5, 1, 0, 7, 4, 5, 0, 7, 6, 4, 0, 7, 2, 6, 0, \
	/* +W */ \
	0, 2, 3, 7, 0, 3, 1, 7, 0, 1, 5, 7, 0, 5, 4, 7, 0, 4, 6, 7, 0, 6, 2, 7 \
}

#define BOX_48_CELL_TEXTURE_MAP_INDICES_COMPACT_2X2X2_GRID TArray<int32> { \
	/* -X */ \
	8, 17, 16, 13, 8, 16, 7, 13, 8, 7, 4, 13, 8, 4, 5, 13, 8, 5, 14, 13, 8, 14, 17, 13, \
	/* +X */ \
	9, 12, 13, 22, 9, 13, 10, 22, 9, 10, 19, 22, 9, 19, 18, 22, 9, 18, 21, 22, 9, 21, 12, 22, \
	/* -Y */ \
	13, 10, 11, 20, 13, 11, 14, 20, 13, 14, 23, 20, 13, 23, 22, 20, 13, 22, 19, 20, 13, 19, 10, 20, \
	/* +Y */ \
	3, 6, 7, 16, 3, 7, 4, 16, 3, 4, 13, 16, 3, 13, 12, 16, 3, 12, 15, 16, 3, 15, 6, 16, \
	/* -Z */ \
	24, 25, 22, 13, 24, 22, 21, 13, 24, 21, 12, 13, 24, 12, 15, 13, 24, 15, 16, 13, 24, 16, 25, 13, \
	/* +Z */ \
	1, 4, 5, 14, 1, 5, 2, 14, 1, 2, 11, 14, 1, 11, 10, 14, 1, 10, 13, 14, 1, 13, 4, 14, \
	/* -W */ \
	13, 22, 23, 26, 13, 25, 22, 26, 13, 16, 25, 26, 13, 17, 16, 26, 13, 14, 17, 26, 13, 23, 14, 26, \
	/* +W */ \
	0, 3, 4, 13, 0, 4, 1, 13, 0, 1, 10, 13, 0, 10, 9, 13, 0, 9, 12, 13, 0, 12, 3, 13 \
}

#define BOX_48_CELL_TEXTURE_MAP_INDICES_LONG_CROSS TArray<int32> { \
	/* -X */ \
	7, 0, 1, 3, 7, 1, 8, 3, 7, 8, 12, 3, 7, 12, 11, 3, 7, 11, 2, 3, 7, 2, 0, 3, \
	/* +X */ \
	19, 23, 24, 31, 19, 24, 20, 31, 19, 20, 29, 31, 19, 29, 28, 31, 19, 28, 30, 31, 19, 30, 23, 31, \
	/* -Y */ \
	17, 20, 19, 7, 17, 19, 16, 7, 17, 16, 4, 7, 17, 4, 5, 7, 17, 5, 8, 7, 17, 8, 20, 7, \
	/* +Y */ \
	11, 14, 15, 27, 11, 15, 12, 27, 11, 12, 24, 27, 11, 24, 23, 27, 11, 23, 26, 27, 11, 26, 14, 27, \
	/* -Z */ \
	7, 6, 10, 22, 7, 10, 11, 22, 7, 11, 23, 22, 7, 23, 19, 22, 7, 19, 18, 22, 7, 18, 6, 22, \
	/* +Z */ \
	8, 12, 13, 25, 8, 13, 9, 25, 8, 9, 21, 25, 8, 21, 20, 25, 8, 20, 24, 25, 8, 24, 12, 25, \
	/* -W */ \
	35, 31, 30, 28, 35, 29, 31, 28, 35, 33, 29, 28, 35, 32, 33, 28, 35, 34, 32, 28, 35, 30, 34, 28, \
	/* +W */ \
	7, 11, 12, 24, 7, 12, 8, 24, 7, 8, 20, 24, 7, 20, 19, 24, 7, 19, 23, 24, 7, 23, 11, 24, \
}
/* clang-format on */
