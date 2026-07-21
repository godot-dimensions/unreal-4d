#include "CylinderShape4DData.h"

double UCylinderShape4DData::GetHypervolume() const
{
	return (4.0 * UE_DOUBLE_PI / 3.0) * (Radius * Radius * Radius * Height);
}

FRaycastResult4D UCylinderShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCylinderShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	// Cylinder raycasting: infinite cylinder along Z axis, truncated by height.
	// See also the similar code in `UCubinderShape4DData::RaycastIntersectsLocal4D`.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const double halfHeight = Height * 0.5;
	// Ray-infinite-cylinder intersection in the XZW plane.
	const FVector4 radialPoint = FVector4(InLocalRayOrigin.X, 0.0, InLocalRayOrigin.Z, InLocalRayOrigin.W);
	const double radialPointLenSq = radialPoint.SizeSquared();
	const double radiusSq = Radius * Radius;
	// Check if starting point is inside the cylinder (radially and vertically).
	const bool startInside = radialPointLenSq <= radiusSq && FMath::Abs(InLocalRayOrigin.Y) <= halfHeight;
	if (bInInsideIsZero && startInside)
	{
		result.bHit = true;
		result.Distance = 0.0;
		result.Normal = FVector4(0.0, 0.0, 0.0, 0.0);
		result.Point = InLocalRayOrigin;
		return result;
	}
	const FVector4 radialDir = FVector4(InLocalRayDirection.X, 0.0, InLocalRayDirection.Z, InLocalRayDirection.W);
	const double radialDirLenSq = radialDir.SizeSquared();
	const double radialPointDotRadialDir = Dot4(radialPoint, radialDir);
	double bestDistance = InMaxDistance;
	// Check if the ray is parallel to the cylinder axis (Y axis).
	if (radialDirLenSq <= UNREAL4D_COMPARISON_EPSILON)
	{
		// Ray moves only along Y axis, parallel to the cylinder axis.
		// Check if we're inside the cylinder radially.
		if (radialPointLenSq <= radiusSq && FMath::Abs(InLocalRayDirection.Y) > UNREAL4D_COMPARISON_EPSILON)
		{
			// We're inside radially, so we'll hit a cap.
			const double distanceToCap = (InLocalRayDirection.Y > 0.0) ? (halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y : (-halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y;
			if (distanceToCap >= 0.0 && distanceToCap < bestDistance)
			{
				const FVector4 normal = (InLocalRayDirection.Y > 0.0) ? FVector4(0, 1, 0, 0) : FVector4(0, -1, 0, 0);
				bestDistance = distanceToCap;
				result.bHit = true;
				result.Distance = distanceToCap;
				result.Normal = normal;
				result.Point = InLocalRayOrigin + InLocalRayDirection * distanceToCap;
			}
		}
		// Regardless of whether we hit the surface or not, return here, because the ray is perpendicular to the curved surface.
		return result;
	}
	const double discriminant = radialPointDotRadialDir * radialPointDotRadialDir - radialDirLenSq * (radialPointLenSq - radiusSq);
	if (discriminant < 0.0)
	{
		return result; // No intersection with infinite cylinder.
	}
	const double sqrtDiscriminant = FMath::Sqrt(discriminant);
	if (startInside)
	{
		// Ray starts inside, use exit point.
		const double distanceToExit = (-radialPointDotRadialDir + sqrtDiscriminant) / radialDirLenSq;
		if (distanceToExit >= 0.0 && distanceToExit < bestDistance)
		{
			const FVector4 exitPoint = InLocalRayOrigin + InLocalRayDirection * distanceToExit;
			if (FMath::Abs(exitPoint.Y) <= halfHeight)
			{
				// Exit through curved surface.
				const FVector4 hitRadial = FVector4(exitPoint.X, 0.0, exitPoint.Z, exitPoint.W);
				const FVector4 normal = hitRadial.GetSafeNormal();
				bestDistance = distanceToExit;
				result.bHit = true;
				result.Distance = distanceToExit;
				result.Normal = normal;
				result.Point = exitPoint;
				return result;
			}
		}
		// Exit point is outside height bounds, check cap.
		if (FMath::Abs(InLocalRayDirection.Y) > UNREAL4D_COMPARISON_EPSILON)
		{
			// Since we started inside, which cap to use is determined by the direction of the ray.
			const double distanceToCap = (InLocalRayDirection.Y > 0.0) ? (halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y : (-halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y;
			if (distanceToCap >= 0.0 && distanceToCap < bestDistance)
			{
				const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToCap;
				const FVector4 capRadial = FVector4(capPoint.X, 0.0, capPoint.Z, capPoint.W);
				if (capRadial.SizeSquared() <= radiusSq)
				{
					const FVector4 normal = (InLocalRayDirection.Y > 0.0) ? FVector4(0, 1, 0, 0) : FVector4(0, -1, 0, 0);
					bestDistance = distanceToCap;
					result.bHit = true;
					result.Distance = distanceToCap;
					result.Normal = normal;
					result.Point = capPoint;
				}
			}
		}
		// Regardless of whether we hit the surface or not, return here, because we started inside and this is the end of that case.
		return result;
	}
	// Ray starts outside.
	// Check both caps.
	if (FMath::Abs(InLocalRayDirection.Y) > UNREAL4D_COMPARISON_EPSILON)
	{
		// Try top cap.
		const double distanceToTopCap = (halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y;
		if (distanceToTopCap >= 0.0 && distanceToTopCap < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToTopCap;
			const FVector4 capRadial = FVector4(capPoint.X, 0.0, capPoint.Z, capPoint.W);
			if (capRadial.SizeSquared() <= radiusSq)
			{
				bestDistance = distanceToTopCap;
				result.bHit = true;
				result.Distance = distanceToTopCap;
				result.Normal = FVector4(0, 1, 0, 0);
				result.Point = capPoint;
			}
		}
		// Try bottom cap.
		const double distanceToBottomCap = (-halfHeight - InLocalRayOrigin.Y) / InLocalRayDirection.Y;
		if (distanceToBottomCap >= 0.0 && distanceToBottomCap < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToBottomCap;
			const FVector4 capRadial = FVector4(capPoint.X, 0.0, capPoint.Z, capPoint.W);
			if (capRadial.SizeSquared() <= radiusSq)
			{
				bestDistance = distanceToBottomCap;
				result.bHit = true;
				result.Distance = distanceToBottomCap;
				result.Normal = FVector4(0, -1, 0, 0);
				result.Point = capPoint;
			}
		}
	}
	// Check curved surface entrance (only if no cap hit or curved surface is closer).
	const double distanceToEntrance = (-radialPointDotRadialDir - sqrtDiscriminant) / radialDirLenSq;
	if (distanceToEntrance >= 0.0)
	{
		const FVector4 entrancePoint = InLocalRayOrigin + InLocalRayDirection * distanceToEntrance;
		if (FMath::Abs(entrancePoint.Y) <= halfHeight && distanceToEntrance < bestDistance)
		{
			// Entrance through curved surface.
			const FVector4 hitRadial = FVector4(entrancePoint.X, 0.0, entrancePoint.Z, entrancePoint.W);
			const FVector4 normal = hitRadial.GetSafeNormal();
			bestDistance = distanceToEntrance;
			result.bHit = true;
			result.Distance = distanceToEntrance;
			result.Normal = normal;
			result.Point = entrancePoint;
		}
	}
	return result;
}

double UCylinderShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const double halfHeight = Height * 0.5;
	FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, InLocalPoint.W);
	const double nearLenSq = nearest.SizeSquared();
	const double radialSignedDistance = FMath::Sqrt(nearLenSq) - Radius;
	const double verticalSignedDistance = FMath::Abs(InLocalPoint.Z) - halfHeight;
	const double absRadialSignedDistance = FMath::Abs(radialSignedDistance);
	const double absVerticalSignedDistance = FMath::Abs(verticalSignedDistance);
	if (OutNearestPointOnSurface != nullptr)
	{
		if (radialSignedDistance > 0.0 || absRadialSignedDistance >= absVerticalSignedDistance)
		{
			if (nearLenSq == 0.0)
			{
				nearest = FVector4(Radius, 0.0, 0.0, 0.0); // Arbitrary point on the surface.
			}
			else
			{
				nearest *= Radius / FMath::Sqrt(nearLenSq);
			}
		}
		if (verticalSignedDistance > 0.0 || absVerticalSignedDistance > absRadialSignedDistance)
		{
			nearest.Z = (InLocalPoint.Z > 0.0) ? halfHeight : -halfHeight;
		}
		else
		{
			nearest.Z = InLocalPoint.Z;
		}
		*OutNearestPointOnSurface = nearest;
	}
	// Return the smallest signed distance, which corresponds to the closest surface.
	if (absVerticalSignedDistance < absRadialSignedDistance)
	{
		return verticalSignedDistance;
	}
	return radialSignedDistance;
}

FVector4 UCylinderShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	const double halfHeight = Height * 0.5;
	FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, InLocalPoint.W);
	const double nearLenSq = nearest.SizeSquared();
	if (nearLenSq > Radius * Radius)
	{
		nearest *= Radius / FMath::Sqrt(nearLenSq);
	}
	if (InLocalPoint.Z > halfHeight)
	{
		nearest.Z = halfHeight;
	}
	else if (InLocalPoint.Z < -halfHeight)
	{
		nearest.Z = -halfHeight;
	}
	else
	{
		nearest.Z = InLocalPoint.Z;
	}
	return nearest;
}

FVector4 UCylinderShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const double halfHeight = Height * 0.5;
	FVector4 support = FVector4(InLocalDirection.X, InLocalDirection.Y, 0.0, InLocalDirection.W).GetSafeNormal() * Radius;
	support.Z = (InLocalDirection.Z > 0.0) ? halfHeight : -halfHeight;
	return support;
}

double UCylinderShape4DData::GetSurfaceVolume() const
{
	return (8.0 * UE_DOUBLE_PI / 3.0) * (Radius * Radius * Radius) + (4.0 * UE_DOUBLE_PI) * (Radius * Radius * Height);
}

bool UCylinderShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	FVector4 absPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	if (absPoint.Z > Height * 0.5)
	{
		return false;
	}
	absPoint.Z = 0.0;
	return absPoint.SizeSquared() <= Radius * Radius;
}

bool UCylinderShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UCylinderShape4DData* cylinderShape = Cast<UCylinderShape4DData>(InShape);
	if (cylinderShape == nullptr)
	{
		return false;
	}
	return Height == cylinderShape->Height && Radius == cylinderShape->Radius;
}
