#include "CapsuleShape4DData.h"

double UCapsuleShape4DData::GetFullHeight() const
{
	if (bHeightIsFull)
	{
		return Height;
	}
	return Height + Radius * 2.0;
}

void UCapsuleShape4DData::SetFullHeight(const double InFullHeight)
{
	if (bHeightIsFull)
	{
		Height = InFullHeight;
	}
	else
	{
		Height = InFullHeight - Radius * 2.0;
	}
}

double UCapsuleShape4DData::GetMidHeight() const
{
	if (bHeightIsFull)
	{
		return Height - Radius * 2.0;
	}
	return Height;
}

void UCapsuleShape4DData::SetMidHeight(const double InMidHeight)
{
	if (bHeightIsFull)
	{
		Height = InMidHeight + Radius * 2.0;
	}
	else
	{
		Height = InMidHeight;
	}
}

void UCapsuleShape4DData::SetHeightIsFull(const bool bInHeightIsFull)
{
	if (bInHeightIsFull == bHeightIsFull) { return; }
	bHeightIsFull = bInHeightIsFull;
	if (bInHeightIsFull)
	{
		// Convert from mid-height to full height.
		Height += Radius * 2.0;
	}
	else
	{
		// Convert from full height to mid-height.
		Height -= Radius * 2.0;
	}
}

double UCapsuleShape4DData::GetHypervolume() const
{
	const double midHeight = GetMidHeight();
	return (0.125 * UNREAL4D_TAU * UNREAL4D_TAU) * (Radius * Radius * Radius * Radius) + (2.0 * UNREAL4D_TAU / 3.0) * (Radius * Radius * Radius * midHeight);
}

double UCapsuleShape4DData::GetSurfaceVolume() const
{
	const double midHeight = GetMidHeight();
	return (0.5 * UNREAL4D_TAU * UNREAL4D_TAU) * (Radius * Radius * Radius) + (2.0 * UNREAL4D_TAU) * (Radius * Radius * midHeight);
}

FRaycastResult4D UCapsuleShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCapsuleShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	if (bInInsideIsZero && HasLocalPoint(InLocalRayOrigin))
	{
		result.bHit = true;
		result.Distance = 0.0;
		result.Normal = FVector4(0.0, 0.0, 0.0, 0.0);
		result.Point = InLocalRayOrigin;
		return result;
	}
	// Capsule raycasting: sweep a sphere along the Z axis.
	// The capsule is a cylinder of height (h - 2r) with hemispherical caps.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const double halfMidHeight = GetMidHeight() * 0.5;
	const double radiusSquared = Radius * Radius;
	double bestDistance = InMaxDistance;
	FVector4 bestNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	// Ray-cylinder intersection (the middle part).
	const FVector4 radialPoint = FVector4(InLocalRayOrigin.X, 0.0, InLocalRayOrigin.Z, InLocalRayOrigin.W);
	const FVector4 radialDir = FVector4(InLocalRayDirection.X, 0.0, InLocalRayDirection.Z, InLocalRayDirection.W);
	const double radialDirLenSq = radialDir.SizeSquared();
	const double radialPointDotRadialDir = Dot4(radialPoint, radialDir);
	const double radialPointLenSq = radialPoint.SizeSquared();
	// Check ray-infinite-cylinder intersection (the middle section).
	if (radialDirLenSq > UNREAL4D_COMPARISON_EPSILON)
	{
		const double discriminant = radialPointDotRadialDir * radialPointDotRadialDir - radialDirLenSq * (radialPointLenSq - radiusSquared);
		if (discriminant >= 0.0)
		{
			const double sqrtDiscriminant = FMath::Sqrt(discriminant);
			const double distanceNear = (-radialPointDotRadialDir - sqrtDiscriminant) / radialDirLenSq;
			const double distanceFar = (-radialPointDotRadialDir + sqrtDiscriminant) / radialDirLenSq;
			const double cylinderDistance = (distanceNear >= 0.0) ? distanceNear : distanceFar;
			if (cylinderDistance >= 0.0)
			{
				const FVector4 hitPoint = InLocalRayOrigin + InLocalRayDirection * cylinderDistance;
				if (FMath::Abs(hitPoint.Z) <= halfMidHeight)
				{
					if (cylinderDistance < bestDistance)
					{
						bestDistance = cylinderDistance;
						const FVector4 hitRadial = FVector4(hitPoint.X, 0.0, hitPoint.Z, hitPoint.W);
						bestNormal = hitRadial.GetSafeNormal();
					}
				}
			}
		}
	}
	// Check ray-sphere intersection at the top cap (center at z = halfMidHeight).
	{
		const FVector4 sphereCenterOffsetTop = InLocalRayOrigin - FVector4(0, 0, halfMidHeight, 0);
		const double directionLengthSquared = InLocalRayDirection.SizeSquared();
		const double directionDotOffsetTop = Dot4(InLocalRayDirection, sphereCenterOffsetTop);
		const double offsetLengthSquaredTop = sphereCenterOffsetTop.SizeSquared();
		const double discriminantTop = directionDotOffsetTop * directionDotOffsetTop - directionLengthSquared * (offsetLengthSquaredTop - radiusSquared);
		if (discriminantTop >= 0.0)
		{
			const double sqrtDiscriminantTop = FMath::Sqrt(discriminantTop);
			const double distanceNearTop = (-directionDotOffsetTop - sqrtDiscriminantTop) / directionLengthSquared;
			const double distanceFarTop = (-directionDotOffsetTop + sqrtDiscriminantTop) / directionLengthSquared;
			// Check near distance first, then far distance.
			double distanceTop = -1.0;
			if (distanceNearTop >= 0.0)
			{
				const FVector4 hitPointNear = InLocalRayOrigin + InLocalRayDirection * distanceNearTop;
				if (hitPointNear.Z >= halfMidHeight)
				{
					distanceTop = distanceNearTop;
				}
			}
			// If near distance didn't work, try far distance.
			if (distanceTop < 0.0 && distanceFarTop >= 0.0)
			{
				const FVector4 hitPointFar = InLocalRayOrigin + InLocalRayDirection * distanceFarTop;
				if (hitPointFar.Z >= halfMidHeight)
				{
					distanceTop = distanceFarTop;
				}
			}
			if (distanceTop >= 0.0 && distanceTop < bestDistance)
			{
				const FVector4 hitPointTop = InLocalRayOrigin + InLocalRayDirection * distanceTop;
				bestDistance = distanceTop;
				bestNormal = (hitPointTop - FVector4(0, 0, halfMidHeight, 0)).GetSafeNormal();
			}
		}
	}
	// Check ray-sphere intersection at the bottom cap (center at z = -halfMidHeight).
	{
		const FVector4 sphereCenterOffsetBottom = InLocalRayOrigin - FVector4(0, 0, -halfMidHeight, 0);
		const double directionLengthSquared = InLocalRayDirection.SizeSquared();
		const double directionDotOffsetBottom = Dot4(InLocalRayDirection, sphereCenterOffsetBottom);
		const double offsetLengthSquaredBottom = sphereCenterOffsetBottom.SizeSquared();
		const double discriminantBottom = directionDotOffsetBottom * directionDotOffsetBottom - directionLengthSquared * (offsetLengthSquaredBottom - radiusSquared);
		if (discriminantBottom >= 0.0)
		{
			const double sqrtDiscriminantBottom = FMath::Sqrt(discriminantBottom);
			const double distanceNearBottom = (-directionDotOffsetBottom - sqrtDiscriminantBottom) / directionLengthSquared;
			const double distanceFarBottom = (-directionDotOffsetBottom + sqrtDiscriminantBottom) / directionLengthSquared;
			// Check near distance first, then far distance.
			double distanceBottom = -1.0;
			if (distanceNearBottom >= 0.0)
			{
				const FVector4 hitPointNear = InLocalRayOrigin + InLocalRayDirection * distanceNearBottom;
				if (hitPointNear.Z <= -halfMidHeight)
				{
					distanceBottom = distanceNearBottom;
				}
			}
			if (distanceBottom < 0.0 && distanceFarBottom >= 0.0)
			{
				// If near distance didn't work, try far distance.
				// Both inside and outside hit at far, so it should be checked after the near check.
				const FVector4 hitPointFar = InLocalRayOrigin + InLocalRayDirection * distanceFarBottom;
				if (hitPointFar.Z <= -halfMidHeight)
				{
					distanceBottom = distanceFarBottom;
				}
			}
			if (distanceBottom >= 0.0 && distanceBottom < bestDistance)
			{
				const FVector4 hitPointBottom = InLocalRayOrigin + InLocalRayDirection * distanceBottom;
				bestDistance = distanceBottom;
				bestNormal = (hitPointBottom - FVector4(0, 0, -halfMidHeight, 0)).GetSafeNormal();
			}
		}
	}
	if (bestDistance < InMaxDistance)
	{
		result.bHit = true;
		result.Distance = bestDistance;
		result.Normal = bestNormal;
		result.Point = InLocalRayOrigin + InLocalRayDirection * bestDistance;
	}
	return result;
}

double UCapsuleShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const double halfMidHeight = GetMidHeight() * 0.5;
	FVector4 sphereRelative = InLocalPoint;
	if (InLocalPoint.Z > halfMidHeight)
	{
		sphereRelative.Z -= halfMidHeight;
	}
	else if (InLocalPoint.Z < -halfMidHeight)
	{
		sphereRelative.Z += halfMidHeight;
	}
	else
	{
		sphereRelative.Z = 0.0;
	}
	const double sphereCenterDistance = sphereRelative.Size();
	if (OutNearestPointOnSurface != nullptr)
	{
		if (sphereCenterDistance == 0.0)
		{
			sphereRelative = FVector4(Radius, 0.0, 0.0, 0.0); // Arbitrary point on the surface.
		}
		else
		{
			sphereRelative *= Radius / sphereCenterDistance;
		}
		if (InLocalPoint.Z > halfMidHeight)
		{
			sphereRelative.Z += halfMidHeight;
		}
		else if (InLocalPoint.Z < -halfMidHeight)
		{
			sphereRelative.Z -= halfMidHeight;
		}
		else
		{
			sphereRelative.Z = InLocalPoint.Z;
		}
		*OutNearestPointOnSurface = sphereRelative;
	}
	return sphereCenterDistance - Radius;
}

FVector4 UCapsuleShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	const double halfMidHeight = GetMidHeight() * 0.5;
	if (InLocalPoint.Z > halfMidHeight)
	{
		FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Z - halfMidHeight, InLocalPoint.Z, InLocalPoint.W);
		const double nearLenSq = nearest.SizeSquared();
		if (nearLenSq > Radius * Radius)
		{
			nearest *= Radius / FMath::Sqrt(nearLenSq);
		}
		nearest.Z += halfMidHeight;
		return nearest;
	}
	if (InLocalPoint.Z < -halfMidHeight)
	{
		FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Z + halfMidHeight, InLocalPoint.Z, InLocalPoint.W);
		const double nearLenSq = nearest.SizeSquared();
		if (nearLenSq > Radius * Radius)
		{
			nearest *= Radius / FMath::Sqrt(nearLenSq);
		}
		nearest.Z -= halfMidHeight;
		return nearest;
	}
	// Point's Z coordinate is within the capsule's height.
	FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, InLocalPoint.W);
	const double nearLenSq = nearest.SizeSquared();
	if (nearLenSq > Radius * Radius)
	{
		nearest *= Radius / FMath::Sqrt(nearLenSq);
	}
	nearest.Z = InLocalPoint.Z;
	return nearest;
}

FVector4 UCapsuleShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const double halfMidHeight = GetMidHeight() * 0.5;
	FVector4 nearest = InLocalDirection.GetSafeNormal() * Radius;
	nearest.Z += (InLocalDirection.Z > 0.0) ? halfMidHeight : -halfMidHeight;
	return nearest;
}

bool UCapsuleShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	FVector4 absPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	const double halfMidHeight = GetMidHeight() * 0.5;
	if (absPoint.Z > halfMidHeight)
	{
		absPoint.Z -= halfMidHeight;
	}
	else
	{
		absPoint.Z = 0.0;
	}
	return absPoint.SizeSquared() <= Radius * Radius;
}

bool UCapsuleShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UCapsuleShape4DData* capsuleShape = Cast<UCapsuleShape4DData>(InShape);
	if (capsuleShape == nullptr)
	{
		return false;
	}
	return Height == capsuleShape->Height && Radius == capsuleShape->Radius && bHeightIsFull == capsuleShape->bHeightIsFull;
}
