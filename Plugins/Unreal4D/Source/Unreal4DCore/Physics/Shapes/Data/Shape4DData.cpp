#include "Shape4DData.h"

FRect4D UShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	// General rect bounds implementation that uses GetSupportPoint to work for all shapes.
	// Some shapes provide their own custom implementation instead (ex: box and orthoplex).
	FRect4D bounds = FRect4D(InTransformToTarget.Origin, FVector4(0.0, 0.0, 0.0, 0.0));
	const FMatrix transposedBasis = InTransformToTarget.Basis.GetTransposed();
	for (int32 sign = 0; sign < 2; sign++)
	{
		for (int32 axis = 0; axis < 4; axis++)
		{
			// Check for a support point in the direction of the transpose's axes.
			// This gives us, after transforming to the target space, the farthest point in the cardinal directions.
			const FVector4 row = UBasis4D::Basis4D_GetRow(transposedBasis, axis);
			FVector4 support = GetSupportPointInLocalDirection(sign == 0 ? -row : row);
			bounds = bounds.ExpandToPoint(InTransformToTarget.TransformPosition(support));
		}
	}
	return bounds;
}

FRaycastResult4D UShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	// Fallback implementation that uses GetRectBoundsLocal4D to work for all shapes.
	const FRect4D localBounds = GetRectBoundsLocal4D(FTransform4D::Identity());
	if (bInInsideIsZero && localBounds.HasPoint(InLocalRayOrigin))
	{
		return FRaycastResult4D::EmptyWithHit();
	}
	return localBounds.RaycastIntersectsFull(InLocalRayOrigin, InLocalRayDirection, InMaxDistance, bInInsideIsZero);
}
