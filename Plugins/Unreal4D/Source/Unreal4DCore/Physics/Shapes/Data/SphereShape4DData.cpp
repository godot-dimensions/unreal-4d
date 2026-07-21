#include "SphereShape4DData.h"

double USphereShape4DData::GetHypervolume() const
{
	return (0.125 * UNREAL4D_TAU * UNREAL4D_TAU) * (Radius * Radius * Radius * Radius);
}

FRaycastResult4D USphereShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("USphereShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	if (bInInsideIsZero && HasLocalPoint(InLocalRayOrigin))
	{
		result.bHit = true;
		result.Point = InLocalRayOrigin;
		return result;
	}
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const double directionLenSq = Dot4(InLocalRayDirection, InLocalRayDirection);
	const double directionDotOrigin = Dot4(InLocalRayDirection, InLocalRayOrigin);
	const double originLenSq = Dot4(InLocalRayOrigin, InLocalRayOrigin);
	const double radiusSquared = Radius * Radius;
	// Discriminant of the quadratic equation.
	const double discriminant = directionDotOrigin * directionDotOrigin - directionLenSq * (originLenSq - radiusSquared);
	if (discriminant < 0.0)
	{
		return result; // No intersection.
	}
	const double sqrtDiscriminant = FMath::Sqrt(discriminant);
	const double distanceNear = (-directionDotOrigin - sqrtDiscriminant) / directionLenSq;
	const double distanceFar = (-directionDotOrigin + sqrtDiscriminant) / directionLenSq;
	double hitDistance = -std::numeric_limits<double>::infinity();
	if (distanceNear >= 0.0)
	{
		// Raycast from the outside hits the sphere at distanceNear.
		hitDistance = distanceNear;
	}
	else if (distanceFar >= 0.0)
	{
		// Both inside and outside hit at distanceFar, so it needs to come in an else after the distanceNear check.
		hitDistance = distanceFar;
	}
	else
	{
		return result; // Ray is pointing away from sphere.
	}
	if (hitDistance >= InMaxDistance)
	{
		return result; // Hit is too far away.
	}
	const FVector4 hitPoint = InLocalRayOrigin + InLocalRayDirection * hitDistance;
	const FVector4 normal = hitPoint.GetSafeNormal(); // Normal is the direction from center to hit point.
	result.bHit = true;
	result.Distance = hitDistance;
	result.Normal = normal;
	result.Point = hitPoint;
	return result;
}

double USphereShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const double lengthSq = InLocalPoint.SizeSquared();
	const double centerDistance = FMath::Sqrt(lengthSq);
	if (OutNearestPointOnSurface != nullptr)
	{
		if (lengthSq == 0.0)
		{
			*OutNearestPointOnSurface = FVector4(Radius, 0.0, 0.0, 0.0); // Arbitrary point on the surface.
		}
		else
		{
			*OutNearestPointOnSurface = InLocalPoint * (Radius / centerDistance);
		}
	}
	return centerDistance - Radius;
}

FVector4 USphereShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	const double lengthSq = InLocalPoint.SizeSquared();
	if (lengthSq <= Radius * Radius)
	{
		return InLocalPoint;
	}
	return InLocalPoint * (Radius / FMath::Sqrt(lengthSq));
}

FVector4 USphereShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	return InLocalDirection.GetSafeNormal() * Radius;
}

double USphereShape4DData::GetSurfaceVolume() const
{
	return (0.5 * UNREAL4D_TAU * UNREAL4D_TAU) * (Radius * Radius * Radius);
}

bool USphereShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	return InLocalPoint.SizeSquared() <= Radius * Radius;
}

bool USphereShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const USphereShape4DData* sphereShape = Cast<USphereShape4DData>(InShape);
	if (sphereShape == nullptr)
	{
		return false;
	}
	return Radius == sphereShape->Radius;
}
