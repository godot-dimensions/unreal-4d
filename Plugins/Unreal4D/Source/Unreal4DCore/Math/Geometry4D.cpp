#include "Geometry4D.h"

#include "VectorFuncs4D.h"

// Barycentric tetrahedra calculations. Don't expose these, it just needs to be efficient
// and shared between TetraMesh4D, ConcaveMeshShape4D, ConvexHullShape4D, etc.

bool UGeometry4D::ComputeInverseMetric3x3(const double InG00, const double InG01, const double InG02, const double InG11, const double InG12, const double InG22, double OutInvSymmetric[6])
{
	// This is the 3x3 symmetric metric matrix G_ij = e_i · e_j for the tetrahedron basis.
	// We store only its six unique entries and invert that packed matrix.
	const double c00 = InG11 * InG22 - InG12 * InG12;
	const double c01 = InG02 * InG12 - InG01 * InG22;
	const double c02 = InG01 * InG12 - InG02 * InG11;
	const double c11 = InG00 * InG22 - InG02 * InG02;
	const double c12 = InG02 * InG01 - InG00 * InG12;
	const double c22 = InG00 * InG11 - InG01 * InG01;
	const double det = InG00 * c00 + InG01 * c01 + InG02 * c02;
	if (!FMath::IsFinite(det) || FMath::IsNearlyEqual(det, 0.0, UNREAL4D_COMPARISON_EPSILON))
	{
		return false;
	}
	const double invDet = 1.0 / det;
	OutInvSymmetric[0] = c00 * invDet;
	OutInvSymmetric[1] = c01 * invDet;
	OutInvSymmetric[2] = c02 * invDet;
	OutInvSymmetric[3] = c11 * invDet;
	OutInvSymmetric[4] = c12 * invDet;
	OutInvSymmetric[5] = c22 * invDet;
	return true;
}

void UGeometry4D::GetNearestPointOnTetrahedronBarycentric(const FVector4& InVert0, const FVector4& InVert1, const FVector4& InVert2, const FVector4& InVert3, const FVector4& InPoint, const TArray<double>& InNearestTetraInverseGramMetricCache, const int64 InTetrahedronIndex, FVector4& OutNearestOnTet, double& OutDistanceSquared, bool& OutProjInside)
{
	OutNearestOnTet = InVert0;
	OutDistanceSquared = UVectorFuncs4D::Vector4_DistanceSquared(InVert0, InPoint);
	OutProjInside = false;
	if (InNearestTetraInverseGramMetricCache.Num() < InTetrahedronIndex * 6 + 6)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UGeometry4D::GetNearestPointOnTetrahedronBarycentric: Inverse metric cache is too small for the given tetrahedron index."));
		ensure(false);
		return;
	}
	const FVector4 edge1 = InVert1 - InVert0;
	const FVector4 edge2 = InVert2 - InVert0;
	const FVector4 edge3 = InVert3 - InVert0;
	// Solve for the barycentric coordinates, which implicitly solves local to the plane of the tetrahedron.
	double bary0, bary1, bary2, bary3;
	{
		const int64 cacheOffset = InTetrahedronIndex * 6;
		const FVector4 local = InPoint - InVert0;
		const double edge1Alignment = Dot4(edge1, local);
		const double edge2Alignment = Dot4(edge2, local);
		const double edge3Alignment = Dot4(edge3, local);
		const double inv00 = InNearestTetraInverseGramMetricCache[cacheOffset + 0];
		const double inv01 = InNearestTetraInverseGramMetricCache[cacheOffset + 1];
		const double inv02 = InNearestTetraInverseGramMetricCache[cacheOffset + 2];
		const double inv11 = InNearestTetraInverseGramMetricCache[cacheOffset + 3];
		const double inv12 = InNearestTetraInverseGramMetricCache[cacheOffset + 4];
		const double inv22 = InNearestTetraInverseGramMetricCache[cacheOffset + 5];
		bary1 = inv00 * edge1Alignment + inv01 * edge2Alignment + inv02 * edge3Alignment;
		bary2 = inv01 * edge1Alignment + inv11 * edge2Alignment + inv12 * edge3Alignment;
		bary3 = inv02 * edge1Alignment + inv12 * edge2Alignment + inv22 * edge3Alignment;
		bary0 = 1.0 - (bary1 + bary2 + bary3);
	}
	// The point is inside the tetrahedron if all barycentric coordinates are non-negative (allowing for a small epsilon).
	const bool projInside = bary0 >= -UNREAL4D_COMPARISON_EPSILON && bary1 >= -UNREAL4D_COMPARISON_EPSILON && bary2 >= -UNREAL4D_COMPARISON_EPSILON && bary3 >= -UNREAL4D_COMPARISON_EPSILON;
	// Determine the nearest point and/or the min distance based on if it's inside or outside the tetrahedron.
	FVector4 nearestOnTet;
	double minDistSq = std::numeric_limits<double>::infinity();
	if (projInside)
	{
		// In this case, the nearest point on the plane lands inside of the tetrahedron.
		nearestOnTet = InVert0 + edge1 * bary1 + edge2 * bary2 + edge3 * bary3;
	}
	else
	{
		// In this case, the nearest point on the plane lands outside, so we need to check the triangle borders.
		const FVector4 nearestOnTri0 = ClosestPointOnTriangle(InVert1, InVert2, InVert3, InPoint);
		const FVector4 nearestOnTri1 = ClosestPointOnTriangle(InVert0, InVert2, InVert3, InPoint);
		const FVector4 nearestOnTri2 = ClosestPointOnTriangle(InVert0, InVert1, InVert3, InPoint);
		const FVector4 nearestOnTri3 = ClosestPointOnTriangle(InVert0, InVert1, InVert2, InPoint);
		nearestOnTet = nearestOnTri0;
		minDistSq = UVectorFuncs4D::Vector4_DistanceSquared(nearestOnTri0, InPoint);
		const double distSqTri1 = UVectorFuncs4D::Vector4_DistanceSquared(nearestOnTri1, InPoint);
		if (distSqTri1 < minDistSq)
		{
			nearestOnTet = nearestOnTri1;
			minDistSq = distSqTri1;
		}
		const double distSqTri2 = UVectorFuncs4D::Vector4_DistanceSquared(nearestOnTri2, InPoint);
		if (distSqTri2 < minDistSq)
		{
			nearestOnTet = nearestOnTri2;
			minDistSq = distSqTri2;
		}
		const double distSqTri3 = UVectorFuncs4D::Vector4_DistanceSquared(nearestOnTri3, InPoint);
		if (distSqTri3 < minDistSq)
		{
			nearestOnTet = nearestOnTri3;
			minDistSq = distSqTri3;
		}
	}
	// Write the outputs.
	OutProjInside = projInside;
	OutNearestOnTet = nearestOnTet;
	if (minDistSq == std::numeric_limits<double>::infinity()) {
		minDistSq = UVectorFuncs4D::Vector4_DistanceSquared(nearestOnTet, InPoint);
	}
	OutDistanceSquared = minDistSq;
}

bool UGeometry4D::IsPointInsideTetrahedronBarycentric(const FVector4& InVert0, const FVector4& InVert1, const FVector4& InVert2, const FVector4& InVert3, const FVector4& InPoint, const TArray<double>& InNearestTetraInverseGramMetricCache, const int64 InTetrahedronIndex)
{
	if (InNearestTetraInverseGramMetricCache.Num() < InTetrahedronIndex * 6 + 6)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UGeometry4D::GetNearestPointOnTetrahedronBarycentric: Inverse metric cache is too small for the given tetrahedron index."));
		ensure(false);
		return false;
	}
	const FVector4 edge1 = InVert1 - InVert0;
	const FVector4 edge2 = InVert2 - InVert0;
	const FVector4 edge3 = InVert3 - InVert0;
	// Solve for the barycentric coordinates, which implicitly solves local to the plane of the tetrahedron.
	double bary0, bary1, bary2, bary3;
	{
		const int64 cacheOffset = InTetrahedronIndex * 6;
		const FVector4 local = InPoint - InVert0;
		const double edge1Alignment = Dot4(edge1, local);
		const double edge2Alignment = Dot4(edge2, local);
		const double edge3Alignment = Dot4(edge3, local);
		const double inv00 = InNearestTetraInverseGramMetricCache[cacheOffset + 0];
		const double inv01 = InNearestTetraInverseGramMetricCache[cacheOffset + 1];
		const double inv02 = InNearestTetraInverseGramMetricCache[cacheOffset + 2];
		const double inv11 = InNearestTetraInverseGramMetricCache[cacheOffset + 3];
		const double inv12 = InNearestTetraInverseGramMetricCache[cacheOffset + 4];
		const double inv22 = InNearestTetraInverseGramMetricCache[cacheOffset + 5];
		bary1 = inv00 * edge1Alignment + inv01 * edge2Alignment + inv02 * edge3Alignment;
		bary2 = inv01 * edge1Alignment + inv11 * edge2Alignment + inv12 * edge3Alignment;
		bary3 = inv02 * edge1Alignment + inv12 * edge2Alignment + inv22 * edge3Alignment;
		bary0 = 1.0 - (bary1 + bary2 + bary3);
	}
	// The point is inside the tetrahedron if all barycentric coordinates are non-negative (allowing for a small epsilon).
	return bary0 >= -UNREAL4D_COMPARISON_EPSILON && bary1 >= -UNREAL4D_COMPARISON_EPSILON && bary2 >= -UNREAL4D_COMPARISON_EPSILON && bary3 >= -UNREAL4D_COMPARISON_EPSILON;
}

// Point-line calculations.

FVector4 UGeometry4D::ClosestPointOnLine(const FVector4& InLinePosition, const FVector4& InLineDirection, const FVector4& InPoint)
{
	const FVector4 vectorToPoint = InPoint - InLinePosition;
	return InLinePosition + UVectorFuncs4D::Vector4_Project(vectorToPoint, InLineDirection);
}

FVector4 UGeometry4D::ClosestPointOnLineSegment(const FVector4& InLineA, const FVector4& InLineB, const FVector4& InPoint)
{
	const FVector4 lineDirection = InLineB - InLineA;
	const FVector4 vectorToPoint = InPoint - InLineA;
	const double dotProduct = Dot4(vectorToPoint, lineDirection);
	const double lineLength = Dot4(lineDirection, lineDirection);
	if (FMath::IsNearlyZero(lineLength))
	{
		return InLineA;
	}
	const double projectionFactor = dotProduct / lineLength;
	if (projectionFactor < 0.0)
	{
		return InLineA;
	}
	else if (projectionFactor > 1.0)
	{
		return InLineB;
	}
	return InLineA + lineDirection * projectionFactor;
}

FVector4 UGeometry4D::ClosestPointOnRay(const FVector4& InRayOrigin, const FVector4& InRayDirection, const FVector4& InPoint)
{
	const FVector4 vectorToPoint = InPoint - InRayOrigin;
	const double dotProduct = Dot4(vectorToPoint, InRayDirection);
	const double rayLength = Dot4(InRayDirection, InRayDirection);
	if (FMath::IsNearlyZero(rayLength))
	{
		return InRayOrigin;
	}
	const double projectionFactor = dotProduct / rayLength;
	if (projectionFactor < 0.0)
	{
		return InRayOrigin;
	}
	return InRayOrigin + InRayDirection * projectionFactor;
}

// Point-shape calculations.

FVector4 UGeometry4D::ClosestPointOnTriangle(const FVector4& InTriangleA, const FVector4& InTriangleB, const FVector4& InTriangleC, const FVector4& InPoint) {
	const FVector4 aToB = InTriangleB - InTriangleA;
	const FVector4 aToC = InTriangleC - InTriangleA;
	const FVector4 aToPoint = InPoint - InTriangleA;
	const double ab_ap = Dot4(aToB, aToPoint);
	const double ac_ap = Dot4(aToC, aToPoint);
	if (ab_ap <= (double)0.0 && ac_ap <= (double)0.0) {
		return InTriangleA;
	}
	const FVector4 bToPoint = InPoint - InTriangleB;
	const double ab_bp = Dot4(aToB, bToPoint);
	const double ac_bp = Dot4(aToC, bToPoint);
	if (ab_bp >= (double)0.0 && ac_bp <= ab_bp) {
		return InTriangleB;
	}
	const double vc = ab_ap * ac_bp - ab_bp * ac_ap;
	if (vc <= (double)0.0 && ab_ap >= (double)0.0 && ab_bp <= (double)0.0) {
		const double bary_edge_ab = ab_ap / (ab_ap - ab_bp);
		return InTriangleA + aToB * bary_edge_ab;
	}
	const FVector4 cToPoint = InPoint - InTriangleC;
	const double ab_cp = Dot4(aToB, cToPoint);
	const double ac_cp = Dot4(aToC, cToPoint);
	if (ac_cp >= (double)0.0 && ab_cp <= ac_cp) {
		return InTriangleC;
	}
	const double vb = ab_cp * ac_ap - ab_ap * ac_cp;
	if (vb <= (double)0.0 && ac_ap >= (double)0.0 && ac_cp <= (double)0.0) {
		const double bary_edge_ac = ac_ap / (ac_ap - ac_cp);
		return InTriangleA + aToC * bary_edge_ac;
	}
	const double va = ab_bp * ac_cp - ab_cp * ac_bp;
	if (va <= (double)0.0 && (ac_bp - ab_bp) >= (double)0.0 && (ab_cp - ac_cp) >= (double)0.0) {
		const FVector4 b_to_c = InTriangleC - InTriangleB;
		const double bary_edge_bc = (ac_bp - ab_bp) / ((ac_bp - ab_bp) + (ab_cp - ac_cp));
		return InTriangleB + b_to_c * bary_edge_bc;
	}
	const double barycentricDenominator = va + vb + vc;
	if (FMath::IsNearlyZero(barycentricDenominator))
	{
		const FVector4 onAB = ClosestPointOnLineSegment(InTriangleA, InTriangleB, InPoint);
		const FVector4 onAC = ClosestPointOnLineSegment(InTriangleA, InTriangleC, InPoint);
		const FVector4 onBC = ClosestPointOnLineSegment(InTriangleB, InTriangleC, InPoint);
		const double distAB = UVectorFuncs4D::Vector4_DistanceSquared(onAB, InPoint);
		const double distAC = UVectorFuncs4D::Vector4_DistanceSquared(onAC, InPoint);
		const double distBC = UVectorFuncs4D::Vector4_DistanceSquared(onBC, InPoint);
		return distAB <= distAC && distAB <= distBC ? onAB : (distAC <= distBC ? onAC : onBC);
	}
	const double denom = 1.0 / barycentricDenominator;
	const double baryAB = vb * denom;
	const double baryAC = vc * denom;
	return InTriangleA + aToB * baryAB + aToC * baryAC;
}

// Line-line calculations.

FVector4 UGeometry4D::ClosestPointBetweenLines(const FVector4& InLine1Point, const FVector4& InLine1Dir, const FVector4& InLine2Point, const FVector4& InLine2Dir)
{
	const FVector4 DifferenceBetweenPoints = InLine1Point - InLine2Point;
	const double Line1LenSq = Dot4(InLine1Dir, InLine1Dir);
	const double Line2LenSq = Dot4(InLine2Dir, InLine2Dir);
	const double Line1Projection = Dot4(InLine1Dir, DifferenceBetweenPoints);
	const double Line2Projection = Dot4(InLine2Dir, DifferenceBetweenPoints);
	const double DirectionDot = Dot4(InLine1Dir, InLine2Dir);
	const double Denominator = Line1LenSq * Line2LenSq - DirectionDot * DirectionDot;
	if (FMath::IsNearlyZero(Denominator, UNREAL4D_COMPARISON_EPSILON))
	{
		// Lines are parallel, handling it as a special case.
		return InLine1Point;
	}
	const double Line1Factor = (DirectionDot * Line2Projection - Line2LenSq * Line1Projection) / Denominator;
	const FVector4 ClosestPointOnLine1 = InLine1Point + InLine1Dir * Line1Factor;
	return ClosestPointOnLine1;
}

FVector4 UGeometry4D::ClosestPointBetweenLineSegments(const FVector4& InLine1A, const FVector4& InLine1B, const FVector4& InLine2A, const FVector4& InLine2B)
{
	const FVector4 DifferenceBetweenPoints = InLine1A - InLine2A;
	const FVector4 Line1Dir = InLine1B - InLine1A;
	const FVector4 Line2Dir = InLine2B - InLine2A;
	const double Line1LenSq = Dot4(Line1Dir, Line1Dir);
	const double Line2LenSq = Dot4(Line2Dir, Line2Dir);
	const double Line1Projection = Dot4(Line1Dir, DifferenceBetweenPoints);
	const double Line2Projection = Dot4(Line2Dir, DifferenceBetweenPoints);
	const double DirectionDot = Dot4(Line1Dir, Line2Dir);
	const double Denominator = Line1LenSq * Line2LenSq - DirectionDot * DirectionDot;
	if (FMath::IsNearlyZero(Denominator, UNREAL4D_COMPARISON_EPSILON))
	{
		// Lines are parallel, handling it as a special case.
		return InLine1A;
	}
	const double Line1Factor = (DirectionDot * Line2Projection - Line2LenSq * Line1Projection) / Denominator;
	const FVector4 ClosestPointOnLine1 = InLine1A + Line1Dir * FMath::Clamp(Line1Factor, 0.0, 1.0);
	return ClosestPointOnLine1;
}

TArray<FVector4> UGeometry4D::ClosestPointsBetweenLines(const FVector4& InLine1Point, const FVector4& InLine1Dir, const FVector4& InLine2Point, const FVector4& InLine2Dir)
{
	const FVector4 DifferenceBetweenPoints = InLine1Point - InLine2Point;
	const double Line1LenSq = Dot4(InLine1Dir, InLine1Dir);
	const double Line2LenSq = Dot4(InLine2Dir, InLine2Dir);
	const double Line1Projection = Dot4(InLine1Dir, DifferenceBetweenPoints);
	const double Line2Projection = Dot4(InLine2Dir, DifferenceBetweenPoints);
	const double DirectionDot = Dot4(InLine1Dir, InLine2Dir);
	const double Denominator = Line1LenSq * Line2LenSq - DirectionDot * DirectionDot;
	if (FMath::IsNearlyZero(Denominator, UNREAL4D_COMPARISON_EPSILON))
	{
		// Lines are parallel, handling it as a special case.
		return TArray<FVector4>{ InLine1Point, InLine2Point };
	}
	const double Line1Factor = (DirectionDot * Line2Projection - Line2LenSq * Line1Projection) / Denominator;
	const double Line2Factor = (Line1LenSq * Line2Projection - DirectionDot * Line1Projection) / Denominator;
	const FVector4 ClosestPointOnLine1 = InLine1Point + InLine1Dir * Line1Factor;
	const FVector4 ClosestPointOnLine2 = InLine2Point + InLine2Dir * Line2Factor;
	return TArray<FVector4>{ ClosestPointOnLine1, ClosestPointOnLine2 };
}

TArray<FVector4> UGeometry4D::ClosestPointsBetweenLineSegments(const FVector4& InLine1A, const FVector4& InLine1B, const FVector4& InLine2A, const FVector4& InLine2B)
{
	const FVector4 DifferenceBetweenPoints = InLine1A - InLine2A;
	const FVector4 Line1Dir = InLine1B - InLine1A;
	const FVector4 Line2Dir = InLine2B - InLine2A;
	const double Line1LenSq = Dot4(Line1Dir, Line1Dir);
	const double Line2LenSq = Dot4(Line2Dir, Line2Dir);
	const double Line1Projection = Dot4(Line1Dir, DifferenceBetweenPoints);
	const double Line2Projection = Dot4(Line2Dir, DifferenceBetweenPoints);
	const double DirectionDot = Dot4(Line1Dir, Line2Dir);
	const double Denominator = Line1LenSq * Line2LenSq - DirectionDot * DirectionDot;
	if (FMath::IsNearlyZero(Denominator, UNREAL4D_COMPARISON_EPSILON))
	{
		// Lines are parallel, handling it as a special case.
		return TArray<FVector4>{ InLine1A, InLine2A };
	}
	const double Line1Factor = (DirectionDot * Line2Projection - Line2LenSq * Line1Projection) / Denominator;
	const double Line2Factor = (Line1LenSq * Line2Projection - DirectionDot * Line1Projection) / Denominator;
	const FVector4 ClosestPointOnLine1 = InLine1A + Line1Dir * FMath::Clamp(Line1Factor, 0.0, 1.0);
	const FVector4 ClosestPointOnLine2 = InLine2A + Line2Dir * FMath::Clamp(Line2Factor, 0.0, 1.0);
	return TArray<FVector4>{ ClosestPointOnLine1, ClosestPointOnLine2 };
}

TArray<FVector4> UGeometry4D::ClosestPointsBetweenLineAndSegment(const FVector4& InLinePoint, const FVector4& InLineDirection, const FVector4& InSegmentA, const FVector4& InSegmentB)
{
	const FVector4 DifferenceBetweenPoints = InLinePoint - InSegmentA;
	const FVector4 SegmentDir = InSegmentB - InSegmentA;
	const double SegmentLenSq = Dot4(SegmentDir, SegmentDir);
	const double LineProjection = Dot4(InLineDirection, DifferenceBetweenPoints);
	const double SegmentProjection = Dot4(SegmentDir, DifferenceBetweenPoints);
	const double DirectionDot = Dot4(InLineDirection, SegmentDir);
	const double LineLenSq = Dot4(InLineDirection, InLineDirection);
	const double Denominator = LineLenSq * SegmentLenSq - DirectionDot * DirectionDot;
	if (FMath::IsNearlyZero(Denominator, UNREAL4D_COMPARISON_EPSILON))
	{
		// Lines are parallel, handling it as a special case.
		return TArray<FVector4>{ InLinePoint, InSegmentA };
	}
	const double LineFactor = (DirectionDot * SegmentProjection - SegmentLenSq * LineProjection) / Denominator;
	const double SegmentFactor = (LineLenSq * SegmentProjection - DirectionDot * LineProjection) / Denominator;
	const FVector4 ClosestPointOnLine = InLinePoint + InLineDirection * LineFactor;
	const FVector4 ClosestPointOnSegment = InSegmentA + SegmentDir * FMath::Clamp(SegmentFactor, 0.0, 1.0);
	return TArray<FVector4>{ ClosestPointOnLine, ClosestPointOnSegment };
}

double UGeometry4D::DistanceBetweenLineAndSegment(const FVector4& InLinePoint, const FVector4& InLineDirection, const FVector4& InSegmentA, const FVector4& InSegmentB)
{
	TArray<FVector4> ClosestPoints = ClosestPointsBetweenLineAndSegment(InLinePoint, InLineDirection, InSegmentA, InSegmentB);
	return (ClosestPoints[0] - ClosestPoints[1]).Size();
}

bool UGeometry4D::BoxContains(const FVector4& InBoxOrigin, const FVector4& InBoxExtents, const FVector4& InPoint)
{
	const FVector4 PointLocalAbs = UVectorFuncs4D::Vector4_Abs(InPoint - InBoxOrigin);

	return (PointLocalAbs.X <= InBoxExtents.X)
		&& (PointLocalAbs.Y <= InBoxExtents.Y)
		&& (PointLocalAbs.Z <= InBoxExtents.Z)
		&& (PointLocalAbs.W <= InBoxExtents.W);
}
