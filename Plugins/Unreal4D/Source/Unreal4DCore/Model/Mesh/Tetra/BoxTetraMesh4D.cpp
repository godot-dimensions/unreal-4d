#include "BoxTetraMesh4D.h"

#include "../Base/BoxMeshData.inc"

const TArray<FVector4> UBoxTetraMesh4D::NormalValues = BOX_CELL_NORMAL_VALUES;
const TArray<FVector> UBoxTetraMesh4D::TextureMapValuesCrossIsland = BOX_TEXTURE_MAP_VALUES_CROSS_ISLAND;
const TArray<FVector> UBoxTetraMesh4D::TextureMapValuesFillEachSide = BOX_TEXTURE_MAP_VALUES_FILL_EACH_SIDE;
const TArray<FVector> UBoxTetraMesh4D::TextureMapValuesCompact2x2x2Grid = BOX_TEXTURE_MAP_VALUES_COMPACT_2X2X2_GRID;
const TArray<FVector> UBoxTetraMesh4D::TextureMapValuesLongCross = BOX_TEXTURE_MAP_VALUES_COMPACT_LONG_CROSS;

const TArray<FVector4> UBoxTetraMesh4D::SimplexCellBoundaryNormals40Cell = BOX_40_CELL_BOUNDARY_NORMALS;
const TArray<int32> UBoxTetraMesh4D::SimplexCellVertexIndices40Cell = BOX_40_CELL_VERTEX_INDICES;
const TArray<int32> UBoxTetraMesh4D::SimplexCellNormalIndices40Cell = BOX_40_CELL_NORMAL_INDICES;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices40CellCrossIsland = BOX_40_CELL_TEXTURE_MAP_INDICES_CROSS_ISLAND;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices40CellFillEachSide = BOX_40_CELL_TEXTURE_MAP_INDICES_FILL_EACH_SIDE;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices40CellCompact2x2x2Grid = BOX_40_CELL_TEXTURE_MAP_INDICES_COMPACT_2X2X2_GRID;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices40CellLongCross = BOX_40_CELL_TEXTURE_MAP_INDICES_LONG_CROSS;

const TArray<FVector4> UBoxTetraMesh4D::SimplexCellBoundaryNormals48Cell = BOX_48_CELL_BOUNDARY_NORMALS;
const TArray<int32> UBoxTetraMesh4D::SimplexCellVertexIndices48Cell = BOX_48_CELL_VERTEX_INDICES;
const TArray<int32> UBoxTetraMesh4D::SimplexCellNormalIndices48Cell = BOX_48_CELL_NORMAL_INDICES;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices48CellCrossIsland = BOX_48_CELL_TEXTURE_MAP_INDICES_CROSS_ISLAND;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices48CellFillEachSide = BOX_48_CELL_TEXTURE_MAP_INDICES_FILL_EACH_SIDE;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices48CellCompact2x2x2Grid = BOX_48_CELL_TEXTURE_MAP_INDICES_COMPACT_2X2X2_GRID;
const TArray<int32> UBoxTetraMesh4D::SimplexCellTextureMapIndices48CellLongCross = BOX_48_CELL_TEXTURE_MAP_INDICES_LONG_CROSS;

FVector4 UBoxTetraMesh4D::GetHalfExtents() const
{
	return Size * 0.5;
}

void UBoxTetraMesh4D::SetHalfExtents(const FVector4& InHalfExtents)
{
	Size = InHalfExtents * 2.0;
	TArray<FVector4> vert;
	vert.SetNum(16);
	vert[0] = FVector4(-InHalfExtents.X, -InHalfExtents.Y, -InHalfExtents.Z, -InHalfExtents.W);
	vert[1] = FVector4(+InHalfExtents.X, -InHalfExtents.Y, -InHalfExtents.Z, -InHalfExtents.W);
	vert[2] = FVector4(-InHalfExtents.X, +InHalfExtents.Y, -InHalfExtents.Z, -InHalfExtents.W);
	vert[3] = FVector4(+InHalfExtents.X, +InHalfExtents.Y, -InHalfExtents.Z, -InHalfExtents.W);
	vert[4] = FVector4(-InHalfExtents.X, -InHalfExtents.Y, +InHalfExtents.Z, -InHalfExtents.W);
	vert[5] = FVector4(+InHalfExtents.X, -InHalfExtents.Y, +InHalfExtents.Z, -InHalfExtents.W);
	vert[6] = FVector4(-InHalfExtents.X, +InHalfExtents.Y, +InHalfExtents.Z, -InHalfExtents.W);
	vert[7] = FVector4(+InHalfExtents.X, +InHalfExtents.Y, +InHalfExtents.Z, -InHalfExtents.W);
	vert[8] = FVector4(-InHalfExtents.X, -InHalfExtents.Y, -InHalfExtents.Z, +InHalfExtents.W);
	vert[9] = FVector4(+InHalfExtents.X, -InHalfExtents.Y, -InHalfExtents.Z, +InHalfExtents.W);
	vert[10] = FVector4(-InHalfExtents.X, +InHalfExtents.Y, -InHalfExtents.Z, +InHalfExtents.W);
	vert[11] = FVector4(+InHalfExtents.X, +InHalfExtents.Y, -InHalfExtents.Z, +InHalfExtents.W);
	vert[12] = FVector4(-InHalfExtents.X, -InHalfExtents.Y, +InHalfExtents.Z, +InHalfExtents.W);
	vert[13] = FVector4(+InHalfExtents.X, -InHalfExtents.Y, +InHalfExtents.Z, +InHalfExtents.W);
	vert[14] = FVector4(-InHalfExtents.X, +InHalfExtents.Y, +InHalfExtents.Z, +InHalfExtents.W);
	vert[15] = FVector4(+InHalfExtents.X, +InHalfExtents.Y, +InHalfExtents.Z, +InHalfExtents.W);
	VertexValuesCache = MoveTemp(vert);
	TetraMeshClearCache();
}

void UBoxTetraMesh4D::SetSize(const FVector4& InSize)
{
	SetHalfExtents(InSize * 0.5);
	Size = InSize; // SetHalfExtents already sets this but this ensures the value set is exactly what was passed in.
}

void UBoxTetraMesh4D::PostLoad()
{
	Super::PostLoad();
	// Rebuild VertexValuesCache from the serialized Size, since VertexValuesCache
	// is not a UPROPERTY and is therefore not persisted.
	SetSize(Size);
}

void UBoxTetraMesh4D::SetTetraDecomp(EBoxTetraDecomp4D InDecomp)
{
	TetraDecomp = InDecomp;
	TetraMeshClearCache();
}

void UBoxTetraMesh4D::SetCellTextureMap(EBoxCellTextureMap4D InMap)
{
	CellTextureMap = InMap;
	TetraMeshClearCache();
}

const TArray<FVector4>& UBoxTetraMesh4D::GetSimplexCellBoundaryNormals() const
{
	return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellBoundaryNormals40Cell : SimplexCellBoundaryNormals48Cell;
}

const TArray<FVector4>& UBoxTetraMesh4D::GetVertexValues() const
{
	ensure(!VertexValuesCache.IsEmpty());
	return VertexValuesCache;
}

const TArray<FVector4>& UBoxTetraMesh4D::GetNormalValues() const
{
	return NormalValues;
}

const TArray<FVector>& UBoxTetraMesh4D::GetTextureMapValues() const
{
	switch (CellTextureMap)
	{
		case EBoxCellTextureMap4D::CrossIsland:
		{
			return TextureMapValuesCrossIsland;
		}
		case EBoxCellTextureMap4D::FillEachSide:
		{
			return TextureMapValuesFillEachSide;
		}
		case EBoxCellTextureMap4D::Compact2x2x2Grid:
		{
			return TextureMapValuesCompact2x2x2Grid;
		}
		case EBoxCellTextureMap4D::LongCross:
		{
			return TextureMapValuesLongCross;
		}
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("Invalid cell texture map type"));
			return EmptyFVectorArray;
		}
	}
}

const TArray<int32>& UBoxTetraMesh4D::GetSimplexCellVertexIndices() const
{
	return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellVertexIndices40Cell : SimplexCellVertexIndices48Cell;
}

const TArray<int32>& UBoxTetraMesh4D::GetSimplexCellNormalIndices() const
{
	return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellNormalIndices40Cell : SimplexCellNormalIndices48Cell;
}

const TArray<int32>& UBoxTetraMesh4D::GetSimplexCellTextureMapIndices() const
{
	switch (CellTextureMap)
	{
		case EBoxCellTextureMap4D::CrossIsland:
		{
			return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellTextureMapIndices40CellCrossIsland : SimplexCellTextureMapIndices48CellCrossIsland;
		}
		case EBoxCellTextureMap4D::FillEachSide:
		{
			return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellTextureMapIndices40CellFillEachSide : SimplexCellTextureMapIndices48CellFillEachSide;
		}
		case EBoxCellTextureMap4D::Compact2x2x2Grid:
		{
			return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellTextureMapIndices40CellCompact2x2x2Grid : SimplexCellTextureMapIndices48CellCompact2x2x2Grid;
		}
		case EBoxCellTextureMap4D::LongCross:
		{
			return TetraDecomp == EBoxTetraDecomp4D::Decomp40Cell ? SimplexCellTextureMapIndices40CellLongCross : SimplexCellTextureMapIndices48CellLongCross;
		}
		default:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("Invalid cell texture map type"));
			return EmptyInt32Array;
		}
	}
}
