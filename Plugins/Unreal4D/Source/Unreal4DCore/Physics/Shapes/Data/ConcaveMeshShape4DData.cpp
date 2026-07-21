#include "ConcaveMeshShape4DData.h"

#include "../../../Math/Geometry4D.h"
#include "../../../Math/Plane4D.h"
#include "../../../Model/Mesh/Tetra/ArrayTetraMesh4D.h"

TArray<FVector4> UConcaveMeshShape4DData::CalculateNormals(const TArray<FVector4>& InSimplexCells)
{
	const int64 normalCount = InSimplexCells.Num() / 4;
	TArray<FVector4> newNormals;
	newNormals.SetNum(normalCount);
	for (int64 i = 0; i < normalCount; i++)
	{
		const FVector4 pivot = InSimplexCells[i * 4];
		const FVector4 a = InSimplexCells[1 + i * 4];
		const FVector4 b = InSimplexCells[2 + i * 4];
		const FVector4 c = InSimplexCells[3 + i * 4];
		const FVector4 perp = UVectorFuncs4D::Vector4_Perpendicular(a - pivot, b - pivot, c - pivot);
		newNormals[i] = perp;
	}
	return newNormals;
}

TArray<double> UConcaveMeshShape4DData::CalculateInverseMetricCache(const TArray<FVector4>& InSimplexCells)
{
	const int64 simplexTetCount = InSimplexCells.Num() / 4;
	TArray<double> newInverseMetricCache;
	newInverseMetricCache.SetNum(simplexTetCount * 6);
	for (int64 simplexTetIndex = 0; simplexTetIndex < simplexTetCount; simplexTetIndex++) {
		// These indices are guaranteed to be within bounds due to mesh validation.
		const FVector4 vert0 = InSimplexCells[simplexTetIndex * 4 + 0];
		const FVector4 vert1 = InSimplexCells[simplexTetIndex * 4 + 1];
		const FVector4 vert2 = InSimplexCells[simplexTetIndex * 4 + 2];
		const FVector4 vert3 = InSimplexCells[simplexTetIndex * 4 + 3];
		const FVector4 edge1 = vert1 - vert0;
		const FVector4 edge2 = vert2 - vert0;
		const FVector4 edge3 = vert3 - vert0;
		const double gram00 = Dot4(edge1, edge1);
		const double gram01 = Dot4(edge1, edge2);
		const double gram02 = Dot4(edge1, edge3);
		const double gram11 = Dot4(edge2, edge2);
		const double gram12 = Dot4(edge2, edge3);
		const double gram22 = Dot4(edge3, edge3);
		double invGram[6];
		const bool valid = UGeometry4D::ComputeInverseMetric3x3(gram00, gram01, gram02, gram11, gram12, gram22, invGram);
		if (!valid) {
			newInverseMetricCache.Reset();
			UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData: Closest-point cache build failed because tetrahedron %lld is degenerate or non-finite."), simplexTetIndex);
			return newInverseMetricCache;
		}
		for (int64 gramIndex = 0; gramIndex < 6; gramIndex++) {
			newInverseMetricCache[simplexTetIndex * 6 + gramIndex] = invGram[gramIndex];
		}
	}
	return newInverseMetricCache;
}

// Thread safety: Consider deferring or synchronizing calls to this function to avoid updating from underneath another thread.
// The contents utilize basic copy-on-write (CoW) semantics, but race conditions are still possible.
void UConcaveMeshShape4DData::SetSimplexCells(const TArray<FVector4>& InSimplexCells)
{
	if (InSimplexCells.Num() % 4 != 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData: Simplexes must be a multiple of 4 (every 4 vertices form a simplex cell). Aborting."));
		return;
	}
	const TArray<FVector4> newNormals = CalculateNormals(InSimplexCells);
	const TArray<double> newInverseMetricCache = CalculateInverseMetricCache(InSimplexCells);
	const int64 simplexTetCount = InSimplexCells.Num() / 4;
	if (newInverseMetricCache.Num() != simplexTetCount * 6)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData: Failed to populate inverse metric cache for the new simplex cells. Aborting."));
		return;
	}
	// Thread safety: Update all of these at once, so that any other thread reading them will see all of them get updated at once.
	// Note that these sequential writes are not guaranteed to be atomic, but it's a very low chance. Consider modifying this if it becomes a problem.
	_simplexCells = InSimplexCells;
	_normals = newNormals;
	_inverseMetricCache = newInverseMetricCache;
}

bool UConcaveMeshShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UConcaveMeshShape4DData* otherShape = Cast<UConcaveMeshShape4DData>(InShape);
	if (otherShape == nullptr)
	{
		return false;
	}
	const TArray<FVector4> otherCells = otherShape->GetSimplexCells();
	return _simplexCells == otherCells;
}

FRaycastResult4D UConcaveMeshShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const {
	// NOTE: bInInsideIsZero is unused in UConcaveMeshShape4DData because concave meshes have no "inside" or "outside" in the same sense as convex shapes.
	// The raycast will always return the closest intersection with any tetrahedron, regardless of whether the ray starts inside or outside the mesh.
	// If handling "inside" vs "outside" is desired, use the cell index returned by the raycast to determine which side of that tetrahedron was hit.
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0, UNREAL4D_COMPARISON_EPSILON))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		ensure(false);
		return result;
	}
	// CoW, cheap layer of thread safety.
	const TArray<double> inverseMetricCache = _inverseMetricCache;
	const TArray<FVector4> simplexCells = _simplexCells;
	const TArray<FVector4> normals = _normals;
	// Check if the data is valid before proceeding with the raycast.
	const int64 simplexTetCount = simplexCells.Num() / 4;
	if (simplexTetCount == 0) {
		return result; // No tetrahedra to raycast against.
	}
	ensure(inverseMetricCache.Num() == simplexTetCount * 6); // Guaranteed to be populated by `SetSimplexCells()`.
	const int64 boundaryNormalsCount = normals.Num();
	ensure(boundaryNormalsCount == simplexTetCount); // Guaranteed to be populated by `SetSimplexCells()`.
	FVector4 bestHitNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistance = InMaxDistance;
	int32 bestTetCellIndex = -1;
	// Iterate through all tetrahedra to find the closest ray intersection.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++) {
		FHyperplane4D tetPlane = FHyperplane4D(normals[tetIndex], simplexCells[tetIndex * 4 + 0]);
		double tetPlaneIntersectionFactor = tetPlane.IntersectRayFactor(InLocalRayFrom, InLocalRayDirection);
		if (tetPlaneIntersectionFactor < 0.0) {
			continue; // No intersection with the plane of this tetrahedron.
		}
		if (tetPlaneIntersectionFactor > bestDistance) {
			continue; // Worse than the best intersection found so far.
		}
		// Check if this candidate intersection is inside the tetrahedron.
		const FVector4 intersectionPoint = InLocalRayFrom + InLocalRayDirection * tetPlaneIntersectionFactor;
		const bool hit = UGeometry4D::IsPointInsideTetrahedronBarycentric(simplexCells[tetIndex * 4], simplexCells[tetIndex * 4 + 1], simplexCells[tetIndex * 4 + 2], simplexCells[tetIndex * 4 + 3], intersectionPoint, inverseMetricCache, tetIndex);
		if (hit) {
			bestHitNormal = tetPlane.Normal;
			bestDistance = tetPlaneIntersectionFactor;
			bestTetCellIndex = tetIndex;
		}
	}
	if (bestTetCellIndex != -1) {
		result.bHit = true;
		result.Distance = bestDistance;
		result.Normal = bestHitNormal;
		result.CellIndex = bestTetCellIndex;
		result.Point = InLocalRayFrom + InLocalRayDirection * bestDistance;
	}
	return result;
}

// NOTE: This algorithm may be sub-optimal for some edge cases with long thin sliver triangles.
// If those edge cases actually become a problem in practice, ask Andrew to improve this code.
double UConcaveMeshShape4DData::GetSignedDistanceToMesh(const FVector4& InLocalPoint, FVector4* OutNearestPointOnTet, int* OutTetrahedronIndex) const {
	// NOTE: In Godot, I used copy-on-write (CoW) for a cheap layer of thread safety.
	// Unreal's TArray type is not copy-on-write, so just reference directly.
	// We could change these to copies later if we need thread safety (just remove the &).
	const TArray<double>& inverseMetricCache = _inverseMetricCache;
	const TArray<FVector4>& simplexCells = _simplexCells;
	const TArray<FVector4>& normals = _normals;
	const int64 simplexTetCount = simplexCells.Num() / 4;
	if (simplexTetCount == 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData: Cannot get nearest point on a mesh shape with zero tetrahedra."));
		return std::numeric_limits<double>::infinity();
	}
	ensure(inverseMetricCache.Num() == simplexTetCount * 6); // Guaranteed to be populated by `set_simplex_cells()`.
	// Iterate over all tetrahedra to find the nearest point on the mesh, keeping track of the best one.
	constexpr int32 MAX_CANDIDATE_TETS = 8;
	FVector4 bestCandidatePointsOnTet[MAX_CANDIDATE_TETS];
	int32 bestCandidateTets[MAX_CANDIDATE_TETS];
	int32 bestCandidateCount = 0;
	FVector4 bestPointOnTet = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistanceSq = std::numeric_limits<double>::infinity();
	int bestTetIndex = -1;
	bool bestProjInside = false;
	// Future: This part could be accelerated with spatial partitioning, and/or accelerated with threading.
	// But those optimizations add a lot of complexity and would only benefit larger meshes.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++) {
		FVector4 nearestOnTet;
		double minDistanceSq = 0.0;
		bool projInside = false;
		const FVector4 vert0 = simplexCells[tetIndex * 4 + 0];
		const FVector4 vert1 = simplexCells[tetIndex * 4 + 1];
		const FVector4 vert2 = simplexCells[tetIndex * 4 + 2];
		const FVector4 vert3 = simplexCells[tetIndex * 4 + 3];
		UGeometry4D::GetNearestPointOnTetrahedronBarycentric(vert0, vert1, vert2, vert3, InLocalPoint, inverseMetricCache, tetIndex, nearestOnTet, minDistanceSq, projInside);
		const bool lessDist = minDistanceSq < bestDistanceSq;
		// If the projection is outside the tet, but the projected point is the same distance as what we
		// already found, then we may have multiple candidates for the closest tet to this point.
		// In this case, we need to collect them all for later disambiguation using the boundary normal.
		if (!projInside) {
			if (!bestProjInside && FMath::IsNearlyEqual(minDistanceSq, bestDistanceSq, UNREAL4D_COMPARISON_EPSILON)) {
				if (likely(bestCandidateCount < MAX_CANDIDATE_TETS)) {
					bestCandidateTets[bestCandidateCount] = tetIndex;
					bestCandidatePointsOnTet[bestCandidateCount] = nearestOnTet;
					bestCandidateCount++;
				} else {
					UE_LOG(LogUnreal4DCore, Warning, TEXT("UConcaveMeshShape4DData: Too many candidate tets for nearest point calculation. Results may be sub-optimal."));
				}
			} else if (lessDist) {
				bestCandidateTets[0] = tetIndex;
				bestCandidatePointsOnTet[0] = nearestOnTet;
				bestCandidateCount = 1;
			}
		}
		// If the projection is closer than what we have already found, then this is the new best point.
		// Update the best point and distance regardless of the projection being inside or outside.
		if (lessDist) {
			bestPointOnTet = nearestOnTet;
			bestDistanceSq = minDistanceSq;
			bestTetIndex = tetIndex;
			bestProjInside = projInside;
			if (projInside) {
				// If the projection is inside, then this is the single unambiguous nearest point so far.
				bestCandidateCount = 0;
			}
		}
	}
	const TArray<FVector4>& boundaryNormals = _normals;
	if (bestCandidateCount > 1) {
		// We have multiple candidates with the same distance, so we need to disambiguate using
		// the absolute angle to the boundary normal (these are normalized, so use the dot product).
		double bestDotAbs = -1.0;
		for (int32 candidateNum = 0; candidateNum < bestCandidateCount; candidateNum++) {
			const int32 candidateTet = bestCandidateTets[candidateNum];
			const FVector4 candidatePointOnTet = bestCandidatePointsOnTet[candidateNum];
			const FVector4 tetPointDirToTarget = (InLocalPoint - candidatePointOnTet).GetSafeNormal();
			const FVector4 normal = boundaryNormals[candidateTet];
			const double dotAbs = FMath::Abs(Dot4(tetPointDirToTarget, normal));
			if (dotAbs > bestDotAbs) {
				bestDotAbs = dotAbs;
				bestTetIndex = candidateTet;
				bestPointOnTet = candidatePointOnTet;
			}
		}
	}
	// Write the outputs.
	if (OutNearestPointOnTet != nullptr) {
		*OutNearestPointOnTet = bestPointOnTet;
	}
	if (OutTetrahedronIndex != nullptr) {
		*OutTetrahedronIndex = bestTetIndex;
	}
	if (unlikely(bestTetIndex < 0)) {
		// This should be impossible because we check for zero tetrahedra above, but just in case.
		return std::numeric_limits<double>::infinity();
	}
	// If we found a nearest point with a nearest tet, check its boundary normal to determine the sign of the distance.
	const FVector4 bestNormal = boundaryNormals[bestTetIndex];
	const double side = Dot4(InLocalPoint - bestPointOnTet, bestNormal);
	double signedDistance = FMath::Sqrt(bestDistanceSq);
	if (side < 0.0) {
		signedDistance = -signedDistance;
	}
	return signedDistance;
}

double UConcaveMeshShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const {
	return GetSignedDistanceToMesh(InLocalPoint, OutNearestPointOnSurface, nullptr);
}

FVector4 UConcaveMeshShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const {
	// NOTE: In Godot, I used copy-on-write (CoW) for a cheap layer of thread safety.
	// Unreal's TArray type is not copy-on-write, so just reference directly.
	// We could change these to copies later if we need thread safety (just remove the &).
	const TArray<double>& inverseMetricCache = _inverseMetricCache;
	const TArray<FVector4>& simplexCells = _simplexCells;
	const TArray<FVector4>& normals = _normals;
	// Check if the data is valid before proceeding with the raycast.
	const int64 simplexTetCount = simplexCells.Num() / 4;
	if (simplexTetCount == 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData: Cannot get nearest point on a mesh shape with zero tetrahedra."));
		return FVector4(0.0, 0.0, 0.0, 0.0);
	}
	ensure(inverseMetricCache.Num() == simplexTetCount * 6); // Guaranteed to be populated by `set_simplex_cells()`.
	// Iterate over all tetrahedra to find the nearest point on the mesh, keeping track of the best one.
	constexpr int32 MAX_CANDIDATE_TETS = 8;
	FVector4 bestCandidatePointsOnTet[MAX_CANDIDATE_TETS];
	int32 bestCandidateTets[MAX_CANDIDATE_TETS];
	int32 bestCandidateTetCount = 0;
	FVector4 bestPointOnTet = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistanceSq = std::numeric_limits<double>::infinity();
	bool bestProjInside = false;
	// Future: This part could be accelerated with spatial partitioning, and/or accelerated with threading.
	// But those optimizations add a lot of complexity and would only benefit larger meshes.
	for (int64 tetIndex = 0; tetIndex < simplexTetCount; tetIndex++) {
		FVector4 nearestOnTet;
		double minDistanceSq = 0.0;
		bool projInside = false;
		const FVector4 vert0 = simplexCells[tetIndex * 4 + 0];
		const FVector4 vert1 = simplexCells[tetIndex * 4 + 1];
		const FVector4 vert2 = simplexCells[tetIndex * 4 + 2];
		const FVector4 vert3 = simplexCells[tetIndex * 4 + 3];
		UGeometry4D::GetNearestPointOnTetrahedronBarycentric(vert0, vert1, vert2, vert3, InLocalPoint, inverseMetricCache, tetIndex, nearestOnTet, minDistanceSq, projInside);
		const bool lessDist = minDistanceSq < bestDistanceSq;
		// If the projection is outside the tet, but the projected point is the same distance as what we
		// already found, then we may have multiple candidates for the closest tet to this point.
		// In this case, we need to collect them all for later disambiguation using the boundary normal.
		if (!projInside) {
			if (!bestProjInside && FMath::IsNearlyEqual(minDistanceSq, bestDistanceSq, UNREAL4D_COMPARISON_EPSILON)) {
				if (bestCandidateTetCount < MAX_CANDIDATE_TETS) {
					bestCandidatePointsOnTet[bestCandidateTetCount] = nearestOnTet;
					bestCandidateTets[bestCandidateTetCount] = tetIndex;
					bestCandidateTetCount++;
				} else {
					UE_LOG(LogUnreal4DCore, Warning, TEXT("UConcaveMeshShape4DData: Too many candidate tets for nearest point calculation. Results may be sub-optimal."));
				}
			} else if (lessDist) {
				bestCandidatePointsOnTet[0] = nearestOnTet;
				bestCandidateTets[0] = tetIndex;
				bestCandidateTetCount = 1;
			}
		}
		// If the projection is closer than what we have already found, then this is the new best point.
		// Update the best point and distance regardless of the projection being inside or outside.
		if (lessDist) {
			bestPointOnTet = nearestOnTet;
			bestDistanceSq = minDistanceSq;
			bestProjInside = projInside;
			if (projInside) {
				// If the projection is inside, then this is the single unambiguous nearest point so far.
				bestCandidateTetCount = 0;
			}
		}
	}
	if (bestCandidateTetCount > 1) {
		// We have multiple candidates with the same distance, so we need to disambiguate using
		// the absolute angle to the boundary normal (these are normalized, so use the dot product).
		double bestDotAbs = -1.0;
		for (int32 candidateNum = 0; candidateNum < bestCandidateTetCount; candidateNum++) {
			const int32 candidateTet = bestCandidateTets[candidateNum];
			const FVector4 candidatePointOnTet = bestCandidatePointsOnTet[candidateNum];
			const FVector4 tetPointDirToTarget = (InLocalPoint - candidatePointOnTet).GetSafeNormal();
			const FVector4 normal = normals[candidateTet];
			const double dotAbs = FMath::Abs(Dot4(tetPointDirToTarget, normal));
			if (dotAbs > bestDotAbs) {
				bestDotAbs = dotAbs;
				bestPointOnTet = candidatePointOnTet;
			}
		}
	}
	return bestPointOnTet;
}

FVector4 UConcaveMeshShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	FVector4 farthestPoint = FVector4(0.0, 0.0, 0.0, 0.0);
	double farthestDistance = -std::numeric_limits<double>::infinity();
	for (const FVector4& vertex : _simplexCells)
	{
		const double distance = Dot4(vertex, InLocalDirection);
		if (distance > farthestDistance)
		{
			farthestDistance = distance;
			farthestPoint = vertex;
		}
	}
	return farthestPoint;
}

UTetraMesh4D* UConcaveMeshShape4DData::ToTetraMesh() const
{
	UArrayTetraMesh4D* mesh = NewObject<UArrayTetraMesh4D>();
	mesh->SetVertexValues(_simplexCells);
	TArray<int32> vertInd;
	vertInd.SetNum(_simplexCells.Num());
	for (int32 i = 0; i < vertInd.Num(); i++)
	{
		vertInd[i] = i;
	}
	mesh->SetSimplexCellVertexIndices(vertInd);
	mesh->SetFlatShadingNormals(true);
	return mesh;
}

void UConcaveMeshShape4DData::SetFromMesh(UTetraMesh4D* InMesh4D)
{
	if (InMesh4D == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UConcaveMeshShape4DData::SetFromMesh: InMesh4D is null."));
		ensure(false);
		return;
	}
	const TArray<int32> simplexCellVertexIndices = InMesh4D->GetSimplexCellVertexIndices();
	const TArray<FVector4> vertices = InMesh4D->GetVertexValues();
	const int32 vertexCount = vertices.Num();
	TArray<FVector4> sampledSimplexPositions;
	for (int32 simplexCellVertexIndex : simplexCellVertexIndices)
	{
		if (simplexCellVertexIndex < 0 || simplexCellVertexIndex >= vertexCount)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("Invalid simplex cell index: %d"), simplexCellVertexIndex);
			break;
		}
		sampledSimplexPositions.Add(vertices[simplexCellVertexIndex]);
	}
	SetSimplexCells(sampledSimplexPositions);
}

UConcaveMeshShape4DData* UConcaveMeshShape4DData::CreateFromMesh(UTetraMesh4D* InMesh)
{
	UConcaveMeshShape4DData* shape = NewObject<UConcaveMeshShape4DData>();
	shape->SetFromMesh(InMesh);
	return shape;
}
