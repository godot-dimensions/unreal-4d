#include "OrthoplexTetraMesh4D.h"

#include "../../../Math/VectorFuncs4D.h"

#include "../Base/OrthoplexMeshData.inc"

const TArray<FVector4> UOrthoplexTetraMesh4D::NormalValuesStatic = ORTHOPLEX_CELL_BOUNDARY_NORMALS;
const TArray<FVector> UOrthoplexTetraMesh4D::TextureMapValuesStatic = ORTHOPLEX_TEXTURE_MAP_VALUES;
const TArray<int32> UOrthoplexTetraMesh4D::SimplexCellVertexIndices = ORTHOPLEX_CELL_VERTEX_INDICES;
const TArray<int32> UOrthoplexTetraMesh4D::SimplexCellNormalIndices = ORTHOPLEX_CELL_NORMAL_INDICES;
const TArray<int32> UOrthoplexTetraMesh4D::SimplexCellTextureMapIndices = ORTHOPLEX_CELL_TEXTURE_MAP_INDICES;

FVector4 UOrthoplexTetraMesh4D::GetHalfExtents() const
{
	return Size * 0.5;
}

void UOrthoplexTetraMesh4D::SetHalfExtents(const FVector4& InHalfExtents)
{
	Size = InHalfExtents * 2.0;
	TArray<FVector4> vert;
	vert.SetNum(8);
	vert[0] = FVector4(-InHalfExtents.X, 0.0, 0.0, 0.0);
	vert[1] = FVector4(+InHalfExtents.X, 0.0, 0.0, 0.0);
	vert[2] = FVector4(0.0, -InHalfExtents.Y, 0.0, 0.0);
	vert[3] = FVector4(0.0, +InHalfExtents.Y, 0.0, 0.0);
	vert[4] = FVector4(0.0, 0.0, -InHalfExtents.Z, 0.0);
	vert[5] = FVector4(0.0, 0.0, +InHalfExtents.Z, 0.0);
	vert[6] = FVector4(0.0, 0.0, 0.0, -InHalfExtents.W);
	vert[7] = FVector4(0.0, 0.0, 0.0, +InHalfExtents.W);
	// Normals can be affected by non-uniform scaling, so we need to recalculate them if the size changes.
	const FVector4 sizeInv = UVectorFuncs4D::Vector4_SafeReciprocal(Size);
	TArray<FVector4> norm;
	norm.SetNum(16);
	norm[0] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[0]);
	norm[1] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[1]);
	norm[2] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[2]);
	norm[3] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[3]);
	norm[4] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[4]);
	norm[5] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[5]);
	norm[6] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[6]);
	norm[7] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[7]);
	norm[8] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[8]);
	norm[9] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[9]);
	norm[10] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[10]);
	norm[11] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[11]);
	norm[12] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[12]);
	norm[13] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[13]);
	norm[14] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[14]);
	norm[15] = UVectorFuncs4D::Vector4_Normalized(sizeInv * NormalValuesStatic[15]);
	VertexValuesCache = MoveTemp(vert);
	NormalValuesCache = MoveTemp(norm);
	TetraMeshClearCache();
}

void UOrthoplexTetraMesh4D::SetSize(const FVector4& InSize)
{
	SetHalfExtents(InSize * 0.5);
	Size = InSize; // SetHalfExtents already sets this but this ensures the value set is exactly what was passed in.
}

void UOrthoplexTetraMesh4D::PostLoad()
{
	Super::PostLoad();
	// Rebuild VertexValuesCache and NormalValuesCache from the serialized Size, since
	// they are not UPROPERTYs and are therefore not persisted.
	SetSize(Size);
}

const TArray<FVector4>& UOrthoplexTetraMesh4D::GetSimplexCellBoundaryNormals() const
{
	ensure(!NormalValuesCache.IsEmpty());
	return NormalValuesCache; // For the orthoplex, these are the same as normal values.
}

const TArray<FVector4>& UOrthoplexTetraMesh4D::GetVertexValues() const
{
	ensure(!VertexValuesCache.IsEmpty());
	return VertexValuesCache;
}

const TArray<FVector4>& UOrthoplexTetraMesh4D::GetNormalValues() const
{
	ensure(!NormalValuesCache.IsEmpty());
	return NormalValuesCache; // For the orthoplex, these are the same as boundary normals.
}

const TArray<FVector>& UOrthoplexTetraMesh4D::GetTextureMapValues() const
{
	return TextureMapValuesStatic;
}

const TArray<int32>& UOrthoplexTetraMesh4D::GetSimplexCellVertexIndices() const
{
	return SimplexCellVertexIndices;
}

const TArray<int32>& UOrthoplexTetraMesh4D::GetSimplexCellNormalIndices() const
{
	return SimplexCellNormalIndices;
}

const TArray<int32>& UOrthoplexTetraMesh4D::GetSimplexCellTextureMapIndices() const
{
	return SimplexCellTextureMapIndices;
}
