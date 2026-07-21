#include "RayShape4DData.h"

FRect4D URayShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	FRect4D rectBounds = FRect4D(InTransformToTarget.Origin, FVector4(0.0, 0.0, 0.0, 0.0));
	const FVector4 rayTargetPoint = UBasis4D::Basis4D_GetRow(InTransformToTarget.Basis, 2) * RayTargetZ + InTransformToTarget.Origin;
	rectBounds.ExpandSelfToPoint(rayTargetPoint);
	return rectBounds;
}

FRaycastResult4D URayShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	// A 1D ray in 4D space will basically never intersect with another 1D ray in 4D space.
	return FRaycastResult4D();
}

double URayShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const FVector4 nearestPoint = GetNearestPointToLocal(InLocalPoint);
	if (OutNearestPointOnSurface != nullptr)
	{
		*OutNearestPointOnSurface = nearestPoint;
	}
	return (InLocalPoint - nearestPoint).Size();
}

FVector4 URayShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	double clampedZ;
	if (likely(RayTargetZ < 0.0))
	{
		clampedZ = FMath::Clamp(InLocalPoint.Z, RayTargetZ, 0.0);
	}
	else
	{
		clampedZ = FMath::Clamp(InLocalPoint.Z, 0.0, RayTargetZ);
	}
	return FVector4(0.0, 0.0, clampedZ, 0.0);
}

FVector4 URayShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	if (likely(RayTargetZ < 0.0) && InLocalDirection.Z > 0.0)
	{
		return FVector4(0.0, 0.0, 0.0, 0.0);
	}
	return FVector4(0, 0, RayTargetZ, 0);
}

bool URayShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	if (!(FMath::IsNearlyZero(InLocalPoint.X, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InLocalPoint.Y, UNREAL4D_COMPARISON_EPSILON) && FMath::IsNearlyZero(InLocalPoint.W, UNREAL4D_COMPARISON_EPSILON)))
	{
		return false;
	}
	double clampedZ;
	if (likely(RayTargetZ < 0.0))
	{
		clampedZ = FMath::Clamp(InLocalPoint.Z, RayTargetZ, 0.0);
	}
	else
	{
		clampedZ = FMath::Clamp(InLocalPoint.Z, 0.0, RayTargetZ);
	}
	return FMath::IsNearlyEqual(InLocalPoint.Z, clampedZ, UNREAL4D_COMPARISON_EPSILON);
}

bool URayShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const URayShape4DData* other = Cast<URayShape4DData>(InShape);
	if (other == nullptr)
	{
		return false;
	}
	return RayTargetZ == other->RayTargetZ;
}
