#include "TetraMesh4D.h"

#include "ArrayTetraMesh4D.h"

#include "../../../Math/Geometry4D.h"
#include "../../../Math/Plane4D.h"
#include "../../../Math/VectorFuncs4D.h"

// Nearest point and signed distance.

void UTetraMesh4D::PopulateInverseMetricCache()
{
	if (!ensureMsgf(IsMeshDataValid(), TEXT("UTetraMesh4D: Cannot populate closest-point cache for an invalid mesh.")))
	{
		_NearestTetraInverseMetricCache.Reset();
		return;
	}
	const TArray<int32>& simplexCellIndices = GetSimplexCellVertexIndices();
	const int64 simplexTetCount = simplexCellIndices.Num() / 4;
	if (_NearestTetraInverseMetricCache.Num() == simplexTetCount * 6)
	{
		return;
	}
	TArray<double> newInverseMetricCache;
	newInverseMetricCache.SetNum(simplexTetCount * 6);
	const TArray<FVector4>& vertices = GetVertexValues();
	for (int64 simplexTetIndex = 0; simplexTetIndex < simplexTetCount; simplexTetIndex++)
	{
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32 i0 = simplexCellIndices[simplexTetIndex * 4 + 0];
		const int32 i1 = simplexCellIndices[simplexTetIndex * 4 + 1];
		const int32 i2 = simplexCellIndices[simplexTetIndex * 4 + 2];
		const int32 i3 = simplexCellIndices[simplexTetIndex * 4 + 3];
		const FVector4 vert0 = vertices[i0];
		const FVector4 edge1 = vertices[i1] - vert0;
		const FVector4 edge2 = vertices[i2] - vert0;
		const FVector4 edge3 = vertices[i3] - vert0;
		const double gram00 = Dot4(edge1, edge1);
		const double gram01 = Dot4(edge1, edge2);
		const double gram02 = Dot4(edge1, edge3);
		const double gram11 = Dot4(edge2, edge2);
		const double gram12 = Dot4(edge2, edge3);
		const double gram22 = Dot4(edge3, edge3);
		double invGram[6];
		const bool valid = UGeometry4D::ComputeInverseMetric3x3(gram00, gram01, gram02, gram11, gram12, gram22, invGram);
		if (!valid)
		{
			newInverseMetricCache.Reset();
			_NearestTetraInverseMetricCache.Reset();
			UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D: Closest-point cache build failed because tetrahedron %d is degenerate or non-finite."), simplexTetIndex);
			return;
		}
		for (int64 gramIndex = 0; gramIndex < 6; gramIndex++)
		{
			newInverseMetricCache[simplexTetIndex * 6 + gramIndex] = invGram[gramIndex];
		}
	}
	_NearestTetraInverseMetricCache = newInverseMetricCache;
}

// NOTE: This algorithm may be sub-optimal for some edge cases with long thin sliver triangles.
// If those edge cases actually become a problem in practice, ask Andrew to improve this code.
double UTetraMesh4D::GetSignedDistanceToMesh(const FVector4& InLocalPoint, FVector4& OutNearestPointOnTet, int& OutTetrahedronIndex, const double InMaxDistance)
{
	OutNearestPointOnTet = FVector4::Zero();
	OutTetrahedronIndex = INDEX_NONE;
	if (!ensureMsgf(InMaxDistance >= 0.0 && FMath::IsFinite(InMaxDistance), TEXT("UTetraMesh4D: Maximum distance must be finite and non-negative.")) ||
		!ensureMsgf(IsMeshDataValid(), TEXT("UTetraMesh4D: Cannot get signed distance to an invalid mesh.")))
	{
		return InMaxDistance;
	}
	const TArray<int32>& simplexCellIndices = GetSimplexCellVertexIndices();
	const int64 simplexTetCount = simplexCellIndices.Num() / 4;
	if (!ensureMsgf(simplexTetCount != 0, TEXT("UTetraMesh4D: Cannot get signed distance to a mesh with zero tetrahedra.")))
	{
		return InMaxDistance;
	}
	if (_NearestTetraInverseMetricCache.Num() != simplexTetCount * 6)
	{
		PopulateInverseMetricCache();
	}
	ensureMsgf(_NearestTetraInverseMetricCache.Num() == simplexTetCount * 6, TEXT("UTetraMesh4D: Closest-point cache is invalid for this mesh."));
	ensureMsgf(simplexTetCount != 0, TEXT("UTetraMesh4D: Cannot get signed distance to a mesh with zero tetrahedra."));
	const TArray<FVector4>& vertices = GetVertexValues();
	// Iterate over all tetrahedra to find the nearest point on the mesh, keeping track of the best one.
	constexpr int32 MAX_CANDIDATE_TETS = 8;
	FVector4 bestCandidatePointsOnTet[MAX_CANDIDATE_TETS];
	int32 bestCandidateTets[MAX_CANDIDATE_TETS];
	int32 bestCandidateCount = 0;
	FVector4 bestPointOnTet = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistanceSq = InMaxDistance;
	int bestTetIndex = -1;
	bool bestProjInside = false;
	// Future: This part could be accelerated with spatial partitioning, and/or accelerated with threading.
	// But those optimizations add a lot of complexity and would only benefit larger meshes.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++)
	{
		FVector4 nearestOnTet;
		double minDistanceSq = 0.0;
		bool projInside = false;
		const FVector4 vert0 = vertices[simplexCellIndices[tetIndex * 4 + 0]];
		const FVector4 vert1 = vertices[simplexCellIndices[tetIndex * 4 + 1]];
		const FVector4 vert2 = vertices[simplexCellIndices[tetIndex * 4 + 2]];
		const FVector4 vert3 = vertices[simplexCellIndices[tetIndex * 4 + 3]];
		UGeometry4D::GetNearestPointOnTetrahedronBarycentric(vert0, vert1, vert2, vert3, InLocalPoint, _NearestTetraInverseMetricCache, tetIndex, nearestOnTet, minDistanceSq, projInside);
		const bool lessDist = minDistanceSq < bestDistanceSq;
		// If the projection is outside the tet, but the projected point is the same distance as what we
		// already found, then we may have multiple candidates for the closest tet to this point.
		// In this case, we need to collect them all for later disambiguation using the boundary normal.
		if (!projInside)
		{
			if (!bestProjInside && FMath::IsNearlyEqual(minDistanceSq, bestDistanceSq, UNREAL4D_COMPARISON_EPSILON))
			{
				if (likely(bestCandidateCount < MAX_CANDIDATE_TETS))
				{
					bestCandidateTets[bestCandidateCount] = tetIndex;
					bestCandidatePointsOnTet[bestCandidateCount] = nearestOnTet;
					bestCandidateCount++;
				}
				else
				{
					UE_LOG(LogUnreal4DCore, Warning, TEXT("UTetraMesh4D: Too many candidate tets for nearest point calculation. Results may be sub-optimal."));
				}
			}
			else if (lessDist)
			{
				bestCandidateTets[0] = tetIndex;
				bestCandidatePointsOnTet[0] = nearestOnTet;
				bestCandidateCount = 1;
			}
		}
		// If the projection is closer than what we have already found, then this is the new best point.
		// Update the best point and distance regardless of the projection being inside or outside.
		if (lessDist)
		{
			bestPointOnTet = nearestOnTet;
			bestDistanceSq = minDistanceSq;
			bestTetIndex = tetIndex;
			bestProjInside = projInside;
			if (projInside)
			{
				// If the projection is inside, then this is the single unambiguous nearest point so far.
				bestCandidateCount = 0;
			}
		}
	}
	const TArray<FVector4>& boundaryNormals = GetSimplexCellBoundaryNormals();
	if (bestCandidateCount > 1)
	{
		// We have multiple candidates with the same distance, so we need to disambiguate using
		// the absolute angle to the boundary normal (these are normalized, so use the dot product).
		double bestDotAbs = -1.0;
		for (int32 candidateNum = 0; candidateNum < bestCandidateCount; candidateNum++)
		{
			const int32 candidateTet = bestCandidateTets[candidateNum];
			const FVector4 candidatePointOnTet = bestCandidatePointsOnTet[candidateNum];
			const FVector4 tetPointDirToTarget = UVectorFuncs4D::Vector4_Normalized(InLocalPoint - candidatePointOnTet);
			const FVector4 normal = boundaryNormals[candidateTet];
			const double dotAbs = FMath::Abs(Dot4(tetPointDirToTarget, normal));
			if (dotAbs > bestDotAbs)
			{
				bestDotAbs = dotAbs;
				bestTetIndex = candidateTet;
				bestPointOnTet = candidatePointOnTet;
			}
		}
	}
	// Write the outputs.
	OutNearestPointOnTet = bestPointOnTet;
	OutTetrahedronIndex = bestTetIndex;
	if (unlikely(bestTetIndex < 0))
	{
		// This should be impossible because we check for zero tetrahedra above, but just in case.
		return InMaxDistance;
	}
	// If we found a nearest point with a nearest tet, check its boundary normal to determine the sign of the distance.
	const FVector4 bestNormal = boundaryNormals[bestTetIndex];
	const double side = Dot4(InLocalPoint - bestPointOnTet, bestNormal);
	double signedDistance = FMath::Sqrt(bestDistanceSq);
	if (side < 0.0)
	{
		signedDistance = -signedDistance;
	}
	return signedDistance;
}

bool UTetraMesh4D::RaycastIntersectsFast(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance)
{
	if (!ensureMsgf(InMaxDistance >= 0.0 && FMath::IsFinite(InMaxDistance), TEXT("UTetraMesh4D: Maximum raycast distance must be finite and non-negative.")) ||
		!ensureMsgf(FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON), TEXT("UTetraMesh4D: Ray direction must be normalized.")) ||
		!ensureMsgf(IsMeshDataValid(), TEXT("UTetraMesh4D: Cannot raycast on an invalid mesh.")))
	{
		return false;
	}
	const TArray<int32>& simplexCellIndices = GetSimplexCellVertexIndices();
	const int64 simplexTetCount = simplexCellIndices.Num() / 4;
	if (simplexTetCount == 0)
	{
		return false; // No tetrahedra to raycast against.
	}
	if (_NearestTetraInverseMetricCache.Num() != simplexTetCount * 6)
	{
		PopulateInverseMetricCache();
	}
	if (!ensureMsgf(_NearestTetraInverseMetricCache.Num() == simplexTetCount * 6, TEXT("UTetraMesh4D: Closest-point cache is invalid for this mesh.")))
	{
		return false;
	}
	const TArray<FVector4>& vertices = GetVertexValues();
	const TArray<FVector4>& boundaryNormals = GetSimplexCellBoundaryNormals();
	const int64 boundaryNormalsCount = boundaryNormals.Num();
	// Iterate through all tetrahedra to find any ray intersection.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++)
	{
		FHyperplane4D tetPlane;
		if (simplexTetCount == boundaryNormalsCount)
		{
			tetPlane = FHyperplane4D(boundaryNormals[tetIndex], vertices[simplexCellIndices[tetIndex * 4 + 0]]);
		}
		else
		{
			// Fallback in case the boundary normals are not available or are mismatched.
			const FVector4& pivot = vertices[simplexCellIndices[tetIndex * 4]];
			const FVector4& a = vertices[simplexCellIndices[1 + tetIndex * 4]];
			const FVector4& b = vertices[simplexCellIndices[2 + tetIndex * 4]];
			const FVector4& c = vertices[simplexCellIndices[3 + tetIndex * 4]];
			const FVector4 perp = UVectorFuncs4D::Vector4_Perpendicular(a - pivot, b - pivot, c - pivot);
			tetPlane = FHyperplane4D(UVectorFuncs4D::Vector4_Normalized(perp), pivot);
		}
		double tetPlaneIntersectionFactor = tetPlane.IntersectRayFactor(InLocalRayFrom, InLocalRayDirection);
		if (tetPlaneIntersectionFactor < 0.0)
		{
			continue; // No intersection with the plane of this tetrahedron.
		}
		if (tetPlaneIntersectionFactor > InMaxDistance)
		{
			continue;
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const FVector4 intersectionPoint = InLocalRayFrom + InLocalRayDirection * tetPlaneIntersectionFactor;
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32 i0 = simplexCellIndices[tetIndex * 4 + 0];
		const int32 i1 = simplexCellIndices[tetIndex * 4 + 1];
		const int32 i2 = simplexCellIndices[tetIndex * 4 + 2];
		const int32 i3 = simplexCellIndices[tetIndex * 4 + 3];
		const FVector4 vert0 = vertices[i0];
		const FVector4 vert1 = vertices[i1];
		const FVector4 vert2 = vertices[i2];
		const FVector4 vert3 = vertices[i3];
		const bool hit = UGeometry4D::IsPointInsideTetrahedronBarycentric(vert0, vert1, vert2, vert3, intersectionPoint, _NearestTetraInverseMetricCache, tetIndex);
		if (hit)
		{
			// For this fast version, we only care if there is any intersection, so we can return true immediately.
			return true;
		}
	}
	return false;
}

FRaycastResult4D UTetraMesh4D::RaycastIntersectsLocal4D(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance)
{
	FRaycastResult4D result;
	if (!ensureMsgf(InMaxDistance >= 0.0 && FMath::IsFinite(InMaxDistance), TEXT("UTetraMesh4D: Maximum raycast distance must be finite and non-negative.")) ||
		!ensureMsgf(IsMeshDataValid(), TEXT("UTetraMesh4D: Cannot raycast on an invalid mesh.")))
	{
		return result;
	}
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		ensure(false);
		return result;
	}
	const TArray<int32>& simplexCellIndices = GetSimplexCellVertexIndices();
	const int64 simplexTetCount = simplexCellIndices.Num() / 4;
	if (simplexTetCount == 0)
	{
		return result; // No tetrahedra to raycast against.
	}
	if (_NearestTetraInverseMetricCache.Num() != simplexTetCount * 6)
	{
		PopulateInverseMetricCache();
	}
	if (!ensureMsgf(_NearestTetraInverseMetricCache.Num() == simplexTetCount * 6, TEXT("UTetraMesh4D: Closest-point cache is invalid for this mesh.")))
	{
		return result;
	}
	const TArray<FVector4>& vertices = GetVertexValues();
	const TArray<FVector4>& boundaryNormals = GetSimplexCellBoundaryNormals();
	const TArray<double>& inverseMetricCache = _NearestTetraInverseMetricCache;
	const int64 boundaryNormalsCount = boundaryNormals.Num();
	FVector4 bestHitNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistance = InMaxDistance;
	int32 bestTetCellIndex = -1;
	// Iterate through all tetrahedra to find the closest ray intersection.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++)
	{
		FHyperplane4D tetPlane;
		if (simplexTetCount == boundaryNormalsCount)
		{
			tetPlane = FHyperplane4D(boundaryNormals[tetIndex], vertices[simplexCellIndices[tetIndex * 4 + 0]]);
		}
		else
		{
			// Fallback in case the boundary normals are not available or are mismatched.
			const FVector4& pivot = vertices[simplexCellIndices[tetIndex * 4]];
			const FVector4& a = vertices[simplexCellIndices[1 + tetIndex * 4]];
			const FVector4& b = vertices[simplexCellIndices[2 + tetIndex * 4]];
			const FVector4& c = vertices[simplexCellIndices[3 + tetIndex * 4]];
			const FVector4 perp = UVectorFuncs4D::Vector4_Perpendicular(a - pivot, b - pivot, c - pivot);
			tetPlane = FHyperplane4D(UVectorFuncs4D::Vector4_Normalized(perp), pivot);
		}
		double tetPlaneIntersectionFactor = tetPlane.IntersectRayFactor(InLocalRayFrom, InLocalRayDirection);
		if (tetPlaneIntersectionFactor < 0.0)
		{
			continue; // No intersection with the plane of this tetrahedron.
		}
		if (tetPlaneIntersectionFactor > bestDistance)
		{
			continue; // Worse than the best intersection found so far.
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const FVector4 intersectionPoint = InLocalRayFrom + InLocalRayDirection * tetPlaneIntersectionFactor;
		// These indices are guaranteed to be within bounds due to mesh validation.
		const int32 i0 = simplexCellIndices[tetIndex * 4 + 0];
		const int32 i1 = simplexCellIndices[tetIndex * 4 + 1];
		const int32 i2 = simplexCellIndices[tetIndex * 4 + 2];
		const int32 i3 = simplexCellIndices[tetIndex * 4 + 3];
		const FVector4 vert0 = vertices[i0];
		const FVector4 vert1 = vertices[i1];
		const FVector4 vert2 = vertices[i2];
		const FVector4 vert3 = vertices[i3];
		const bool hit = UGeometry4D::IsPointInsideTetrahedronBarycentric(vert0, vert1, vert2, vert3, intersectionPoint, _NearestTetraInverseMetricCache, tetIndex);
		if (hit)
		{
			bestHitNormal = tetPlane.Normal;
			bestDistance = tetPlaneIntersectionFactor;
			bestTetCellIndex = tetIndex;
		}
	}
	if (bestTetCellIndex != -1)
	{
		result.bHit = true;
		result.Distance = bestDistance;
		result.Normal = bestHitNormal;
		result.Point = InLocalRayFrom + InLocalRayDirection * bestDistance;
		result.CellIndex = bestTetCellIndex;
	}
	return result;
}

FTetraMesh4DRenderResourcePtr UTetraMesh4D::CreateRenderResource()
{
	if (!IsMeshDataValid())
	{
		ensureMsgf(false, TEXT("Cannot create render resource for invalid mesh data in '%s'."), *GetPathName());
		return nullptr;
	}
	FTetraMesh4DRenderResourcePtr newRenderResource = FTetraMesh4DRenderResource::MakeTetraMesh4DRenderResource();
	newRenderResource->InitResources(
		GetVertexValues(), GetNormalValues(), GetTextureMapValues(),
		GetSimplexCellVertexIndices(), GetSimplexCellNormalIndices(), GetSimplexCellTextureMapIndices());
	return newRenderResource;
}

void UTetraMesh4D::TetraMeshClearCache()
{
	FScopeLock lock(&TetraMeshModifyOrCacheLock);
	_NearestTetraInverseMetricCache.Reset();
	MeshClearCaches();
}

bool UTetraMesh4D::ValidateMeshData()
{
	const TArray<int32>& simplexCellVertexIndices = GetSimplexCellVertexIndices();
	const int32 cellVertexIndicesCount = simplexCellVertexIndices.Num();
	if (cellVertexIndicesCount % 4 != 0)
	{
		return false; // Must be a multiple of 4.
	}
	const TArray<FVector4>& simplexCellBoundaryNormals = GetSimplexCellBoundaryNormals();
	const int32 cellBoundaryNormalsCount = simplexCellBoundaryNormals.Num();
	if (cellBoundaryNormalsCount > 0 && cellBoundaryNormalsCount * 4 != cellVertexIndicesCount)
	{
		return false; // Must have one normal per cell (4 indices).
	}
	const TArray<int32>& simplexCellNormalIndices = GetSimplexCellNormalIndices();
	const int32 cellNormalIndicesCount = simplexCellNormalIndices.Num();
	if (cellNormalIndicesCount > 0 && cellNormalIndicesCount != cellVertexIndicesCount)
	{
		return false; // Must have either no normals or one normal index per vertex index.
	}
	const TArray<int32>& simplexCellTextureMapIndices = GetSimplexCellTextureMapIndices();
	const int32 cellTextureMapIndicesCount = simplexCellTextureMapIndices.Num();
	if (cellTextureMapIndicesCount > 0 && cellTextureMapIndicesCount != cellVertexIndicesCount)
	{
		return false; // Must have either no texture map or one texture map index per vertex index.
	}
	const TArray<FVector4>& vertexValues = GetVertexValues();
	const int32 vertexValueCount = vertexValues.Num();
	for (const int32 vertexIndex : simplexCellVertexIndices)
	{
		if (vertexIndex < 0 || vertexIndex >= vertexValueCount)
		{
			return false; // Vertex indices must reference valid vertices.
		}
	}
	const TArray<FVector4>& normalValues = GetNormalValues();
	const int32 normalValueCount = normalValues.Num();
	for (const int32 normalIndex : simplexCellNormalIndices)
	{
		if (normalIndex < 0 || normalIndex >= normalValueCount)
		{
			return false; // Normal indices must reference valid normals.
		}
	}
	const TArray<FVector>& textureMapValues = GetTextureMapValues();
	const int32 textureMapValueCount = textureMapValues.Num();
	for (const int32 textureMapIndex : simplexCellTextureMapIndices)
	{
		if (textureMapIndex < 0 || textureMapIndex >= textureMapValueCount)
		{
			return false; // Texture map indices must reference valid texture map values.
		}
	}
	return true;
}

void UTetraMesh4D::ValidateMaterialForMesh(UMaterial4D* InMaterial)
{
	if (InMaterial == nullptr)
	{
		return;
	}
	const EColorSourceFlags4D albedoSourceFlags = InMaterial->GetAlbedoSourceFlags();
	if (EnumHasAnyFlags(albedoSourceFlags, EColorSourceFlags4D::PerCell))
	{
		const TArray<int32>& vertexIndices = GetSimplexCellVertexIndices();
		const int32 cellCount = vertexIndices.Num() / 4;
		if (InMaterial->AlbedoColorArray.Num() < cellCount)
		{
			InMaterial->ResizeAlbedoColorArray(cellCount);
		}
	}
	UMesh4D::ValidateMaterialForMesh(InMaterial);
}

UTetraMaterial4D* UTetraMesh4D::FallbackMaterial = nullptr;

UMaterial4D* UTetraMesh4D::GetFallbackMaterial() const
{
	return FallbackMaterial;
}

void UTetraMesh4D::InitFallbackMaterial()
{
	if (!FallbackMaterial)
	{
		FallbackMaterial = NewObject<UTetraMaterial4D>(GetTransientPackage());
		FallbackMaterial->AddToRoot();
	}
}

void UTetraMesh4D::CleanupFallbackMaterial()
{
	if (FallbackMaterial)
	{
		FallbackMaterial->RemoveFromRoot();
		FallbackMaterial = nullptr;
	}
}

UArrayTetraMesh4D* UTetraMesh4D::ToArrayTetraMesh()
{
	UArrayTetraMesh4D* arrayMesh = NewObject<UArrayTetraMesh4D>();
	if (arrayMesh)
	{
		arrayMesh->SetVertexValues(GetVertexValues());
		arrayMesh->SetNormalValues(GetNormalValues());
		arrayMesh->SetTextureMapValues(GetTextureMapValues());
		arrayMesh->SetSimplexCellVertexIndices(GetSimplexCellVertexIndices());
		arrayMesh->SetSimplexCellBoundaryNormals(GetSimplexCellBoundaryNormals());
		arrayMesh->SetSimplexCellNormalIndices(GetSimplexCellNormalIndices());
		arrayMesh->SetSimplexCellTextureMapIndices(GetSimplexCellTextureMapIndices());
		arrayMesh->SetMaterial(GetMaterial());
	}
	return arrayMesh;
}

UTetraMesh4D* UTetraMesh4D::ToTetraMesh()
{
	return ToArrayTetraMesh();
}

const TArray<FVector4>& UTetraMesh4D::GetSimplexCellBoundaryNormals() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D::GetSimplexCellBoundaryNormals: This method should be overridden in derived classes."));
	return EmptyFVector4Array;
}

const TArray<int32>& UTetraMesh4D::GetSimplexCellVertexIndices() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D::GetSimplexCellVertexIndices: This method should be overridden in derived classes."));
	return EmptyInt32Array;
}

const TArray<int32>& UTetraMesh4D::GetSimplexCellNormalIndices() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D::GetSimplexCellNormalIndices: This method should be overridden in derived classes."));
	return EmptyInt32Array;
}

const TArray<int32>& UTetraMesh4D::GetSimplexCellTextureMapIndices() const
{
	UE_LOG(LogUnreal4DCore, Error, TEXT("UTetraMesh4D::GetSimplexCellTextureMapIndices: This method should be overridden in derived classes."));
	return EmptyInt32Array;
}
