#include "PlaneShape4DData.h"

#include "../../../Math/Plane4D.h"

FRect4D UPlaneShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	const FVector4 normal = UBasis4D::Basis4D_GetRow(InTransformToTarget.Basis, 2).GetSafeNormal();
	FVector4 rectStart = FVector4(PLANE_START, PLANE_START, PLANE_START, PLANE_START);
	FVector4 rectSize = FVector4(PLANE_WIDTH, PLANE_WIDTH, PLANE_WIDTH, PLANE_WIDTH);
	if (FMath::IsNearlyZero(normal.X))
	{
		if (FMath::IsNearlyZero(normal.Y))
		{
			if (FMath::IsNearlyZero(normal.Z))
			{
				if (!FMath::IsNearlyZero(normal.W))
				{
					rectSize.W = PLANE_THICKNESS;
					if (normal.W > 0.0)
					{
						rectStart.W = InTransformToTarget.Origin.W - PLANE_THICKNESS;
					}
					else
					{
						rectStart.W = InTransformToTarget.Origin.W;
					}
				}
			}
			else if (FMath::IsNearlyZero(normal.W))
			{
				rectSize.Z = PLANE_THICKNESS;
				if (normal.Z > 0.0)
				{
					rectStart.Z = InTransformToTarget.Origin.Z - PLANE_THICKNESS;
				}
				else
				{
					rectStart.Z = InTransformToTarget.Origin.Z;
				}
			}
		}
		else if (FMath::IsNearlyZero(normal.Z) && FMath::IsNearlyZero(normal.W))
		{
			rectSize.Y = PLANE_THICKNESS;
			if (normal.Y > 0.0)
			{
				rectStart.Y = InTransformToTarget.Origin.Y - PLANE_THICKNESS;
			}
			else
			{
				rectStart.Y = InTransformToTarget.Origin.Y;
			}
		}
	}
	else if (FMath::IsNearlyZero(normal.Y) && FMath::IsNearlyZero(normal.Z) && FMath::IsNearlyZero(normal.W))
	{
		rectSize.X = PLANE_THICKNESS;
		if (normal.X > 0.0)
		{
			rectStart.X = InTransformToTarget.Origin.X - PLANE_THICKNESS;
		}
		else
		{
			rectStart.X = InTransformToTarget.Origin.X;
		}
	}
	return FRect4D(rectStart, rectSize);
}

FRaycastResult4D UPlaneShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPlaneShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	if (bInInsideIsZero && InLocalRayOrigin.Z <= 0.0)
	{
		result.bHit = true;
		result.Distance = 0.0;
		result.Normal = FVector4(0.0, 0.0, 0.0, 0.0);
		result.Point = InLocalRayOrigin;
		return result;
	}
	// The plane has its normal pointing in the local +Z direction and intersects the origin.
	const FHyperplane4D plane = FHyperplane4D(FVector4(0, 0, 1, 0), 0.0);
	const double factor = plane.IntersectRayFactor(InLocalRayOrigin, InLocalRayDirection);
	const bool hit = factor >= 0.0 && factor < InMaxDistance;
	result.bHit = hit;
	if (hit)
	{
		result.Distance = factor;
		result.Normal = InLocalRayOrigin.Z > 0.0 ? FVector4(0, 0, 1, 0) : FVector4(0, 0, -1, 0);
		result.Point = InLocalRayOrigin + InLocalRayDirection * factor;
	}
	return result;
}

double UPlaneShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	if (OutNearestPointOnSurface != nullptr)
	{
		*OutNearestPointOnSurface = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, InLocalPoint.W);
	}
	return InLocalPoint.Z;
}

FVector4 UPlaneShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	if (InLocalPoint.Z <= 0.0)
	{
		return InLocalPoint;
	}
	return FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, InLocalPoint.W);
}

FVector4 UPlaneShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	FVector4 supportPoint = FVector4(0.0, 0.0, 0.0, 0.0);
	const double epsilon = KINDA_SMALL_NUMBER;
	if (InLocalDirection.X > UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.X = PLANE_WIDTH;
	}
	else if (InLocalDirection.X < -UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.X = -PLANE_WIDTH;
	}
	if (InLocalDirection.Y > UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.Y = PLANE_WIDTH;
	}
	else if (InLocalDirection.Y < -UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.Y = -PLANE_WIDTH;
	}
	if (InLocalDirection.W > UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.W = PLANE_WIDTH;
	}
	else if (InLocalDirection.W < -UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.W = -PLANE_WIDTH;
	}
	if (InLocalDirection.Z < -UNREAL4D_COMPARISON_EPSILON)
	{
		supportPoint.Z = -PLANE_THICKNESS;
	}
	return supportPoint;
}

bool UPlaneShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	return InLocalPoint.Z <= 0.0;
}

bool UPlaneShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UPlaneShape4DData* other = Cast<UPlaneShape4DData>(InShape);
	// UPlaneShape4DData has no properties, so any valid UPlaneShape4DData is equal to another.
	return other != nullptr;
}
