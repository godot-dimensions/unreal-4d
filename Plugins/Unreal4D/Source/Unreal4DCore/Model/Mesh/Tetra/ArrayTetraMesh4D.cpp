#include "ArrayTetraMesh4D.h"

#include "../../../Math/VectorFuncs4D.h"

bool UArrayTetraMesh4D::ValidateMeshData()
{
	const int32 cellVertexIndicesCount = SimplexCellVertexIndices.Num();
	if (cellVertexIndicesCount % 4 != 0)
	{
		return false; // Must be a multiple of 4.
	}
	const int32 cellBoundaryNormalsCount = SimplexCellBoundaryNormals.Num();
	if (cellBoundaryNormalsCount > 0 && cellBoundaryNormalsCount * 4 != cellVertexIndicesCount)
	{
		return false; // Must have one normal per cell (4 indices).
	}
	const int32 cellNormalIndicesCount = SimplexCellNormalIndices.Num();
	if (cellNormalIndicesCount > 0 && cellNormalIndicesCount != cellVertexIndicesCount)
	{
		return false; // Must have either no normals or one normal index per vertex index.
	}
	const int32 cellTextureMapIndicesCount = SimplexCellTextureMapIndices.Num();
	if (cellTextureMapIndicesCount > 0 && cellTextureMapIndicesCount != cellVertexIndicesCount)
	{
		return false; // Must have either no texture map or one texture map index per vertex index.
	}
	const int32 vertexValueCount = VertexValues.Num();
	for (const int32 vertexIndex : SimplexCellVertexIndices)
	{
		if (vertexIndex < 0 || vertexIndex >= vertexValueCount)
		{
			return false; // Vertex indices must reference valid vertices.
		}
	}
	const int32 normalValueCount = NormalValues.Num();
	for (const int32 normalIndex : SimplexCellNormalIndices)
	{
		if (normalIndex < 0 || normalIndex >= normalValueCount)
		{
			return false; // Normal indices must reference valid normals.
		}
	}
	const int32 textureMapValueCount = TextureMapValues.Num();
	for (const int32 textureMapIndex : SimplexCellTextureMapIndices)
	{
		if (textureMapIndex < 0 || textureMapIndex >= textureMapValueCount)
		{
			return false; // Texture map indices must reference valid texture map values.
		}
	}
	return true;
}

// Append functions.

void UArrayTetraMesh4D::AppendTetraCellPoints(const FVector4& InA, const FVector4& InB, const FVector4& InC, const FVector4& InD, bool bInDeduplicateVertices)
{
	const int32 indexA = AppendVertex(InA, bInDeduplicateVertices);
	const int32 indexB = AppendVertex(InB, bInDeduplicateVertices);
	const int32 indexC = AppendVertex(InC, bInDeduplicateVertices);
	const int32 indexD = AppendVertex(InD, bInDeduplicateVertices);
	AppendTetraCellIndices(indexA, indexB, indexC, indexD);
}

void UArrayTetraMesh4D::AppendTetraCellIndices(int32 InIndexA, int32 InIndexB, int32 InIndexC, int32 InIndexD)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SimplexCellVertexIndices.Add(InIndexA);
	SimplexCellVertexIndices.Add(InIndexB);
	SimplexCellVertexIndices.Add(InIndexC);
	SimplexCellVertexIndices.Add(InIndexD);
	TetraMeshClearCache();
	ResetMeshDataValidation();
}

int32 UArrayTetraMesh4D::AppendVertex_NoLock(const FVector4& InVertex, bool bInDeduplicateVertices)
{
	const int32 vertexValueCount = VertexValues.Num();
	if (bInDeduplicateVertices)
	{
		for (int32 i = 0; i < vertexValueCount; i++)
		{
			if (VertexValues[i] == InVertex)
			{
				return i;
			}
		}
	}
	if (VertexValues.Num() >= MAX_VERTICES)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("Cannot append vertex: vertex limit exceeded"));
		return MAX_VERTICES;
	}
	VertexValues.Add(InVertex);
	return vertexValueCount;
}

int32 UArrayTetraMesh4D::AppendVertex(const FVector4& InVertex, bool bInDeduplicateVertices)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	const int32 newVertIndex = AppendVertex_NoLock(InVertex, bInDeduplicateVertices);
	TetraMeshClearCache();
	ResetMeshDataValidation();
	return newVertIndex;
}

TArray<int32> UArrayTetraMesh4D::AppendVertices(const TArray<FVector4>& InVertices, bool bInDeduplicateVertices)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	TArray<int32> indices;
	for (int32 i = 0; i < InVertices.Num(); i++)
	{
		indices.Add(AppendVertex_NoLock(InVertices[i], bInDeduplicateVertices));
	}
	TetraMeshClearCache();
	ResetMeshDataValidation();
	return indices;
}

// Misc functions.

void UArrayTetraMesh4D::MergeWith(UArrayTetraMesh4D* InOther, const FTransform4D& InTransform)
{
	if (InOther == nullptr || InOther == this || !InOther->IsMeshDataValid() || !IsMeshDataValid())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UArrayTetraMesh4D: Cannot merge invalid meshes."));
		return;
	}
	// Lock both meshes in a consistent order to avoid deadlocks.
	// For example if one thread calls A.MergeWith(B) and another calls B.MergeWith(A) this would cause a deadlock otherwise.
	FCriticalSection* firstLock = &ArrayMeshModifyOrCacheLock;
	FCriticalSection* secondLock = &InOther->ArrayMeshModifyOrCacheLock;
	if (reinterpret_cast<UPTRINT>(firstLock) > reinterpret_cast<UPTRINT>(secondLock))
	{
		Swap(firstLock, secondLock);
	}
	FScopeLock FirstScopeLock(firstLock);
	FScopeLock SecondScopeLock(secondLock);
	const FMatrix normalTransform = UBasis4D::Basis4D_Inverse(InTransform.Basis).GetTransposed();
	// Ensure that if the second mesh has normals, the first mesh has normals.
	// Note: This MUST be done BEFORE merging vertices, because `SetFlatShadingNormals()` uses the vertex indices.
	int32 startNormalIndexCount = SimplexCellNormalIndices.Num();
	int32 otherNormalIndexCount = InOther->SimplexCellNormalIndices.Num();
	if (startNormalIndexCount == 0 && otherNormalIndexCount > 0)
	{
		SetFlatShadingNormals_NoLock();
		startNormalIndexCount = SimplexCellNormalIndices.Num();
	}
	// Merge vertices.
	const int32 startVertexIndexCount = SimplexCellVertexIndices.Num();
	const int32 otherVertexIndexCount = InOther->SimplexCellVertexIndices.Num();
	const int32 endVertexIndexCount = startVertexIndexCount + otherVertexIndexCount;
	const int32 startCellCount = startVertexIndexCount / 4;
	const int32 otherCellCount = otherVertexIndexCount / 4;
	if (startVertexIndexCount > 0 || otherVertexIndexCount > 0)
	{
		// Merge the vertex position values by appending the other's vertex position values transformed by the provided transform.
		const int32 startVertexValueCount = VertexValues.Num();
		const int32 otherVertexValueCount = InOther->VertexValues.Num();
		const int32 endVertexValueCount = startVertexValueCount + otherVertexValueCount;
		VertexValues.SetNum(endVertexValueCount);
		for (int32 i = 0; i < otherVertexValueCount; i++)
		{
			VertexValues[startVertexValueCount + i] = InTransform * InOther->VertexValues[i];
		}
		// Merge the vertex indices by adding the start vertex value count to the other's vertex indices.
		SimplexCellVertexIndices.SetNum(endVertexIndexCount);
		for (int32 i = 0; i < otherVertexIndexCount; i++)
		{
			SimplexCellVertexIndices[startVertexIndexCount + i] = InOther->SimplexCellVertexIndices[i] + startVertexValueCount;
		}
	}
	// Merge normals.
	if (startNormalIndexCount > 0 || otherNormalIndexCount > 0)
	{
		// If only one mesh has normals, calculate flat shading normals for the other mesh.
		if (otherNormalIndexCount == 0)
		{
			InOther->SetFlatShadingNormals_NoLock();
			otherNormalIndexCount = InOther->SimplexCellNormalIndices.Num();
		}
		// Merge the normal values by appending the other's normal values transformed by the basis of the provided transform.
		const int32 startNormalValueCount = NormalValues.Num();
		const int32 otherNormalValueCount = InOther->NormalValues.Num();
		const int32 endNormalValueCount = startNormalValueCount + otherNormalValueCount;
		NormalValues.SetNum(endNormalValueCount);
		for (int32 i = 0; i < otherNormalValueCount; i++)
		{
			NormalValues[startNormalValueCount + i] = UVectorFuncs4D::Vector4_Normalized(
				normalTransform.TransformFVector4(InOther->NormalValues[i]));
		}
		// Merge the normal indices by adding the start normal value count to the other's normal indices.
		const int32 endNormalIndexCount = startNormalIndexCount + otherNormalIndexCount;
		ensure(endNormalIndexCount == endVertexIndexCount); // If present, the normal indices should line up with the vertex indices.
		SimplexCellNormalIndices.SetNum(endNormalIndexCount);
		for (int32 i = 0; i < otherNormalIndexCount; i++)
		{
			SimplexCellNormalIndices[startNormalIndexCount + i] = InOther->SimplexCellNormalIndices[i] + startNormalValueCount;
		}
	}
	// Merge simplex cell boundary normals.
	int32 startBoundaryNormalCount = SimplexCellBoundaryNormals.Num();
	int32 otherBoundaryNormalCount = InOther->SimplexCellBoundaryNormals.Num();
	if (startBoundaryNormalCount > 0 || otherBoundaryNormalCount > 0)
	{
		// If only one mesh has boundary normals, calculate them for the other mesh.
		if (startBoundaryNormalCount == 0 && startCellCount > 0)
		{
			CalculateBoundaryNormals_NoLock();
			startBoundaryNormalCount = SimplexCellBoundaryNormals.Num();
		}
		if (otherBoundaryNormalCount == 0 && otherCellCount > 0)
		{
			InOther->CalculateBoundaryNormals_NoLock();
			otherBoundaryNormalCount = InOther->SimplexCellBoundaryNormals.Num();
		}
		const int32 endBoundaryNormalCount = startBoundaryNormalCount + otherBoundaryNormalCount;
		ensure(startBoundaryNormalCount == startCellCount);
		ensure(otherBoundaryNormalCount == otherCellCount);
		ensure(endBoundaryNormalCount == endVertexIndexCount / 4);
		SimplexCellBoundaryNormals.SetNum(endBoundaryNormalCount);
		for (int32 i = 0; i < otherBoundaryNormalCount; i++)
		{
			SimplexCellBoundaryNormals[startBoundaryNormalCount + i] = UVectorFuncs4D::Vector4_Normalized(
				normalTransform.TransformFVector4(InOther->SimplexCellBoundaryNormals[i]));
		}
	}
	// Merge texture maps.
	int32 startTextureMapIndexCount = SimplexCellTextureMapIndices.Num();
	int32 otherTextureMapIndexCount = InOther->SimplexCellTextureMapIndices.Num();
	if (startTextureMapIndexCount > 0 || otherTextureMapIndexCount > 0)
	{
		// If only one mesh has texture map indices, add default indices that point to a zero texture map value for the other mesh.
		if (startTextureMapIndexCount == 0)
		{
			const int32 zeroValueIndex = TextureMapValues.Add(FVector::Zero());
			SimplexCellTextureMapIndices.SetNum(startVertexIndexCount);
			for (int32 i = 0; i < startVertexIndexCount; i++)
			{
				SimplexCellTextureMapIndices[i] = zeroValueIndex;
			}
			startTextureMapIndexCount = startVertexIndexCount;
		}
		if (otherTextureMapIndexCount == 0)
		{
			const int32 zeroValueIndex = InOther->TextureMapValues.Add(FVector::Zero());
			InOther->SimplexCellTextureMapIndices.SetNum(otherVertexIndexCount);
			for (int32 i = 0; i < otherVertexIndexCount; i++)
			{
				InOther->SimplexCellTextureMapIndices[i] = zeroValueIndex;
			}
			otherTextureMapIndexCount = otherVertexIndexCount;
		}
		// Merge the texture map values by appending the other's texture map values (no transformation).
		const int32 startTextureMapValueCount = TextureMapValues.Num();
		const int32 otherTextureMapValueCount = InOther->TextureMapValues.Num();
		const int32 endTextureMapValueCount = startTextureMapValueCount + otherTextureMapValueCount;
		TextureMapValues.SetNum(endTextureMapValueCount);
		for (int32 i = 0; i < otherTextureMapValueCount; i++)
		{
			TextureMapValues[startTextureMapValueCount + i] = InOther->TextureMapValues[i];
		}
		// Merge the texture map indices by adding the start texture map value count to the other's texture map indices.
		const int32 endTextureMapIndexCount = startTextureMapIndexCount + otherTextureMapIndexCount;
		ensure(endTextureMapIndexCount == endVertexIndexCount); // If present, the texture map indices should line up with the vertex indices.
		SimplexCellTextureMapIndices.SetNum(endTextureMapIndexCount);
		for (int32 i = 0; i < otherTextureMapIndexCount; i++)
		{
			SimplexCellTextureMapIndices[startTextureMapIndexCount + i] = InOther->SimplexCellTextureMapIndices[i] + startTextureMapValueCount;
		}
	}
	// Merge materials.
	UMaterial4D* otherMaterial = InOther->GetMaterial();
	if (otherMaterial)
	{
		UMaterial4D* selfMaterial = GetMaterial();
		if (selfMaterial)
		{
			selfMaterial->MergeWith(otherMaterial, startVertexIndexCount, otherVertexIndexCount);
		}
		else if (otherMaterial->AlbedoColorArray.Num() > 0)
		{
			UMaterial4D* newMaterial = NewObject<UMaterial4D>();
			if (newMaterial)
			{
				newMaterial->MergeWith(otherMaterial, startVertexIndexCount, otherVertexIndexCount);
				SetMaterial(newMaterial);
			}
		}
		else
		{
			SetMaterial(otherMaterial);
		}
	}
	TetraMeshClearCache();
	ResetMeshDataValidation();
}

bool UArrayTetraMesh4D::OverwriteFromOtherMesh(UMesh4D* InFromMesh)
{
	if (InFromMesh == nullptr || InFromMesh == this) { return false; }
	// This function is currently only called by the import code, and expects the passed-in
	// mesh to not be modified by other threads, so the InFromMesh is not completely locked.
	// If this becomes a problem in the future we could add some more locking complexity here.
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	// All meshes have values.
	VertexValues = InFromMesh->GetVertexValues();
	NormalValues = InFromMesh->GetNormalValues();
	TextureMapValues = InFromMesh->GetTextureMapValues();
	Material = InFromMesh->GetMaterial();
	// Tetra meshes define how tetrahedra use those values.
	UTetraMesh4D* inputTetraMesh = Cast<UTetraMesh4D>(InFromMesh);
	if (inputTetraMesh != nullptr)
	{
		SimplexCellVertexIndices = inputTetraMesh->GetSimplexCellVertexIndices();
		SimplexCellBoundaryNormals = inputTetraMesh->GetSimplexCellBoundaryNormals();
		SimplexCellNormalIndices = inputTetraMesh->GetSimplexCellNormalIndices();
		SimplexCellTextureMapIndices = inputTetraMesh->GetSimplexCellTextureMapIndices();
		if (SimplexCellBoundaryNormals.IsEmpty() && !SimplexCellVertexIndices.IsEmpty())
		{
			// Keep importer behavior robust even if the source tetra mesh omitted boundary normals.
			CalculateBoundaryNormals_NoLock();
		}
	}
	TetraMeshClearCache();
	ResetMeshDataValidation();
	return IsMeshDataValid();
}

void UArrayTetraMesh4D::CalculateBoundaryNormals_NoLock(bool bInKeepExisting)
{
	const int32 cellCount = SimplexCellVertexIndices.Num() / 4;
	TArray<FVector4> boundaryNormals;
	boundaryNormals.SetNum(cellCount);
	for (int32 i = 0; i < cellCount; i++)
	{
		if (bInKeepExisting && SimplexCellBoundaryNormals.IsValidIndex(i) && SimplexCellBoundaryNormals[i] != FVector4::Zero())
		{
			boundaryNormals[i] = SimplexCellBoundaryNormals[i];
			continue;
		}
		const FVector4 pivot = VertexValues[SimplexCellVertexIndices[i * 4]];
		const FVector4 a = VertexValues[SimplexCellVertexIndices[1 + i * 4]];
		const FVector4 b = VertexValues[SimplexCellVertexIndices[2 + i * 4]];
		const FVector4 c = VertexValues[SimplexCellVertexIndices[3 + i * 4]];
		// Calculate perpendicular vector in 4D space.
		const FVector4 perp = UVectorFuncs4D::Vector4_Perpendicular(a - pivot, b - pivot, c - pivot);
		boundaryNormals[i] = UVectorFuncs4D::Vector4_Normalized(perp);
	}
	SimplexCellBoundaryNormals = MoveTemp(boundaryNormals);
	TetraMeshClearCache();
}

void UArrayTetraMesh4D::SetFlatShadingNormals_NoLock(bool bInForceRecalculateBoundaryNormals)
{
	const int32 cellIndCount = SimplexCellVertexIndices.Num();
	const int32 cellCount = cellIndCount / 4;
	if (bInForceRecalculateBoundaryNormals || SimplexCellBoundaryNormals.Num() != cellCount)
	{
		CalculateBoundaryNormals_NoLock();
	}
	// For flat shading, the normal values are the same as the boundary normals.
	NormalValues = SimplexCellBoundaryNormals;
	TArray<int32> cellNormIndices;
	cellNormIndices.SetNum(cellIndCount);
	for (int32 cellIndex = 0; cellIndex < cellCount; cellIndex++)
	{
		cellNormIndices[cellIndex * 4 + 0] = cellIndex;
		cellNormIndices[cellIndex * 4 + 1] = cellIndex;
		cellNormIndices[cellIndex * 4 + 2] = cellIndex;
		cellNormIndices[cellIndex * 4 + 3] = cellIndex;
	}
	SimplexCellNormalIndices = MoveTemp(cellNormIndices);
	ResetMeshDataValidation();
}

void UArrayTetraMesh4D::CalculateBoundaryNormals(bool bInKeepExisting)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	CalculateBoundaryNormals_NoLock(bInKeepExisting);
}

void UArrayTetraMesh4D::SetFlatShadingNormals(bool bInForceRecalculateBoundaryNormals)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SetFlatShadingNormals_NoLock(bInForceRecalculateBoundaryNormals);
}

// Standalone data, not sampled by indices.

const TArray<FVector4>& UArrayTetraMesh4D::GetSimplexCellBoundaryNormals() const
{
	ensure(SimplexCellBoundaryNormals.IsEmpty() || SimplexCellBoundaryNormals.Num() * 4 == SimplexCellVertexIndices.Num());
	return SimplexCellBoundaryNormals;
}

void UArrayTetraMesh4D::SetSimplexCellBoundaryNormals(const TArray<FVector4>& InSimplexCellBoundaryNormals)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SimplexCellBoundaryNormals = InSimplexCellBoundaryNormals;
	TetraMeshClearCache();
	ResetMeshDataValidation();
}

// Data which is sampled by indices.

const TArray<FVector4>& UArrayTetraMesh4D::GetVertexValues() const
{
	return VertexValues;
}

void UArrayTetraMesh4D::SetVertexValues(const TArray<FVector4>& InVertexValues)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	if (InVertexValues.Num() > MAX_VERTICES)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("Cannot set vertices: vertex limit exceeded"));
		return;
	}
	VertexValues = InVertexValues;
	TetraMeshClearCache();
	ResetMeshDataValidation();
}

const TArray<FVector4>& UArrayTetraMesh4D::GetNormalValues() const
{
	return NormalValues;
}

void UArrayTetraMesh4D::SetNormalValues(const TArray<FVector4>& InNormalValues)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	NormalValues = InNormalValues;
	ResetMeshDataValidation();
}

const TArray<FVector>& UArrayTetraMesh4D::GetTextureMapValues() const
{
	return TextureMapValues;
}

void UArrayTetraMesh4D::SetTextureMapValues(const TArray<FVector>& InTextureMapValues)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	TextureMapValues = InTextureMapValues;
	ResetMeshDataValidation();
}

// Indices which use that data.

const TArray<int32>& UArrayTetraMesh4D::GetSimplexCellVertexIndices() const
{
	return SimplexCellVertexIndices;
}

void UArrayTetraMesh4D::SetSimplexCellVertexIndices(const TArray<int32>& InVertexIndices)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SimplexCellVertexIndices = InVertexIndices;
	TetraMeshClearCache();
	ResetMeshDataValidation();
}

const TArray<int32>& UArrayTetraMesh4D::GetSimplexCellNormalIndices() const
{
	ensure(SimplexCellNormalIndices.IsEmpty() || SimplexCellNormalIndices.Num() == SimplexCellVertexIndices.Num());
	return SimplexCellNormalIndices;
}

void UArrayTetraMesh4D::SetSimplexCellNormalIndices(const TArray<int32>& InNormalIndices)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SimplexCellNormalIndices = InNormalIndices;
	ResetMeshDataValidation();
}

const TArray<int32>& UArrayTetraMesh4D::GetSimplexCellTextureMapIndices() const
{
	ensure(SimplexCellTextureMapIndices.IsEmpty() || SimplexCellTextureMapIndices.Num() == SimplexCellVertexIndices.Num());
	return SimplexCellTextureMapIndices;
}

void UArrayTetraMesh4D::SetSimplexCellTextureMapIndices(const TArray<int32>& InTextureMapIndices)
{
	FScopeLock ScopeLock(&ArrayMeshModifyOrCacheLock);
	SimplexCellTextureMapIndices = InTextureMapIndices;
	ResetMeshDataValidation();
}

void UArrayTetraMesh4D::PostLoad()
{
	Super::PostLoad();
	ensure(IsMeshDataValid());
	if (SimplexCellBoundaryNormals.IsEmpty())
	{
		CalculateBoundaryNormals_NoLock();
	}
	if (SimplexCellNormalIndices.IsEmpty())
	{
		SetFlatShadingNormals_NoLock();
	}
	ensure(IsMeshDataValid());
}
