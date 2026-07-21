#include "CubinderShape4DData.h"

FVector2D UCubinderShape4DData::GetSizeZW() const
{
	return FVector2D(Height, Thickness); // (Z, W)
}

void UCubinderShape4DData::SetSizeZW(const FVector2D& InSizeZW) {
	Height = InSizeZW.X; // Z
	Thickness = InSizeZW.Y; // W
}

double UCubinderShape4DData::GetHypervolume() const
{
	// http://hi.gher.space/wiki/Cubinder
	return UE_DOUBLE_PI * Radius * Radius * Height * Thickness;
}

double UCubinderShape4DData::GetSurfaceVolume() const
{
	// Wiki describes the surcell volume as 2π*r(r*h + h^2) but that only applies when height == thickness.
	// Distributing gives us 2π*h*r^2 + 2π*r*h^2, where:
	// * The 2π*r*h^2 term is the surcell volume of the square prism, 2π*r long with a h*h base (square).
	// * The 2π*h*r^2 term is the surcell volume of the cylinders, h long with an 2π*r^2 base (circle).
	// With different height and thickness, 2π*r*h^2 becomes 2π*r*h*t, and the cylinders become π*h*r^2 + π*t*r^2 so π*r^2 * (h+t).
	// Check: Try substituting t with h and it should collapse to the original formulas.
	return UNREAL4D_TAU * Radius * Height * Thickness + UE_DOUBLE_PI * Radius * Radius * (Height + Thickness);
}

FRaycastResult4D UCubinderShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UCubinderShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	// Cubinder: circle in XY plane, extruded in Z and W.
	// See also the similar code in `UCylinderShape4DData::RaycastIntersectsLocal4D`.
	// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
	const double halfHeight = Height * 0.5;
	const double halfThickness = Thickness * 0.5;
	// Ray-infinite-cylinder intersection in the XY plane.
	const FVector4 radialPoint = FVector4(InLocalRayOrigin.X, InLocalRayOrigin.Y, 0.0, 0.0);
	const double radialPointLenSq = radialPoint.SizeSquared();
	const double radiusSq = Radius * Radius;
	// Check if starting point is inside the cubinder (radially and within ZW bounds).
	const bool startInside = radialPointLenSq <= radiusSq && FMath::Abs(InLocalRayOrigin.Z) <= halfHeight && FMath::Abs(InLocalRayOrigin.W) <= halfThickness;
	if (bInInsideIsZero && startInside)
	{
		result.bHit = true;
		result.Distance = 0.0;
		result.Normal = FVector4(0.0, 0.0, 0.0, 0.0);
		result.Point = InLocalRayOrigin;
		return result;
	}
	const FVector4 radialDir = FVector4(InLocalRayDirection.X, InLocalRayDirection.Y, 0.0, 0.0);
	const double radialDirLenSq = radialDir.SizeSquared();
	const double radialPointDotRadialDir = Dot4(radialPoint, radialDir);
	FVector4 bestNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistance = InMaxDistance;
	// Check if the ray is parallel to the cubinder axes (Y and/or W axes).
	if (radialDirLenSq <= UNREAL4D_COMPARISON_EPSILON)
	{
		// Ray moves only along Z and/or W axes, parallel to the cubinder axis.
		// Check if we're inside the cylinder radially.
		if (radialPointLenSq <= radiusSq && (FMath::Abs(InLocalRayDirection.Z) > UNREAL4D_COMPARISON_EPSILON || FMath::Abs(InLocalRayDirection.W) > UNREAL4D_COMPARISON_EPSILON))
		{
			// We're inside radially, so we'll hit a Z or W cap.
			// Check Z caps.
			if (FMath::Abs(InLocalRayDirection.Z) > UNREAL4D_COMPARISON_EPSILON)
			{
				const double distanceToZCap = (InLocalRayDirection.Z > 0.0) ? (halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z : (-halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z;
				if (distanceToZCap >= 0.0 && distanceToZCap < bestDistance)
				{
					const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToZCap;
					if (FMath::Abs(capPoint.W) <= halfThickness)
					{
						bestDistance = distanceToZCap;
						bestNormal = (InLocalRayDirection.Z > 0.0) ? FVector4(0, 0, 1, 0) : FVector4(0, 0, -1, 0);
					}
				}
			}
			// Check W caps.
			if (FMath::Abs(InLocalRayDirection.W) > UNREAL4D_COMPARISON_EPSILON)
			{
				const double distanceToWCap = (InLocalRayDirection.W > 0.0) ? (halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W : (-halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W;
				if (distanceToWCap >= 0.0 && distanceToWCap < bestDistance)
				{
					const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToWCap;
					if (FMath::Abs(capPoint.Z) <= halfHeight)
					{
						bestDistance = distanceToWCap;
						bestNormal = (InLocalRayDirection.W > 0.0) ? FVector4(0, 0, 0, 1) : FVector4(0, 0, 0, -1);
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
		}
		// Either we hit or we didn't, but in either case, we return here, because the ray is parallel to the cubinder axis and cannot hit the curved surface.
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
		if (distanceToExit >= 0.0)
		{
			const FVector4 exitPoint = InLocalRayOrigin + InLocalRayDirection * distanceToExit;
			if (FMath::Abs(exitPoint.Z) <= halfHeight && FMath::Abs(exitPoint.W) <= halfThickness)
			{
				// Exit through curved surface.
				const FVector4 hitRadial = FVector4(exitPoint.X, exitPoint.Y, 0.0, 0.0);
				const FVector4 normal = hitRadial.GetSafeNormal();
				bestDistance = distanceToExit;
				result.bHit = true;
				result.Distance = distanceToExit;
				result.Normal = normal;
				result.Point = exitPoint;
				return result;
			}
		}
		// Exit point is outside ZW bounds, check rectangular caps.
		if (FMath::Abs(InLocalRayDirection.Z) > UNREAL4D_COMPARISON_EPSILON)
		{
			const double distanceToZCap = (InLocalRayDirection.Z > 0.0) ? (halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z : (-halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z;
			if (distanceToZCap >= 0.0 && distanceToZCap < bestDistance)
			{
				const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToZCap;
				const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
				if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.W) <= halfThickness)
				{
					bestDistance = distanceToZCap;
					bestNormal = (InLocalRayDirection.Z > 0.0) ? FVector4(0, 0, 1, 0) : FVector4(0, 0, -1, 0);
				}
			}
		}
		if (FMath::Abs(InLocalRayDirection.W) > UNREAL4D_COMPARISON_EPSILON)
		{
			const double distanceToWCap = (InLocalRayDirection.W > 0.0) ? (halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W : (-halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W;
			if (distanceToWCap >= 0.0 && distanceToWCap < bestDistance)
			{
				const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToWCap;
				const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
				if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.Z) <= halfHeight)
				{
					bestDistance = distanceToWCap;
					bestNormal = (InLocalRayDirection.W > 0.0) ? FVector4(0, 0, 0, 1) : FVector4(0, 0, 0, -1);
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
		// Regardless of whether we hit the surface or not, return here, because we started inside and this is the end of that case.
		return result;
	}
	// Ray starts outside.
	// Check all four rectangular caps.
	if (FMath::Abs(InLocalRayDirection.Z) > UNREAL4D_COMPARISON_EPSILON)
	{
		// Try Z caps (positive and negative).
		const double distanceToZCapPos = (halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z;
		if (distanceToZCapPos >= 0.0 && distanceToZCapPos < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToZCapPos;
			const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
			if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.W) <= halfThickness)
			{
				bestDistance = distanceToZCapPos;
				bestNormal = FVector4(0, 0, 1, 0);
				result.bHit = true;
				result.Distance = distanceToZCapPos;
				result.Normal = bestNormal;
			}
		}
		const double distanceToZCapNeg = (-halfHeight - InLocalRayOrigin.Z) / InLocalRayDirection.Z;
		if (distanceToZCapNeg >= 0.0 && distanceToZCapNeg < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToZCapNeg;
			const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
			if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.W) <= halfThickness)
			{
				bestDistance = distanceToZCapNeg;
				bestNormal = FVector4(0, 0, -1, 0);
				result.bHit = true;
				result.Distance = distanceToZCapNeg;
				result.Normal = bestNormal;
			}
		}
	}
	if (FMath::Abs(InLocalRayDirection.W) > UNREAL4D_COMPARISON_EPSILON)
	{
		// Try W caps (positive and negative).
		const double distanceToWCapPos = (halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W;
		if (distanceToWCapPos >= 0.0 && distanceToWCapPos < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToWCapPos;
			const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
			if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.Z) <= halfHeight)
			{
				bestDistance = distanceToWCapPos;
				bestNormal = FVector4(0, 0, 0, 1);
				result.bHit = true;
				result.Distance = distanceToWCapPos;
				result.Normal = bestNormal;
				result.Point = capPoint;
			}
		}
		const double distanceToWCapNeg = (-halfThickness - InLocalRayOrigin.W) / InLocalRayDirection.W;
		if (distanceToWCapNeg >= 0.0 && distanceToWCapNeg < bestDistance)
		{
			const FVector4 capPoint = InLocalRayOrigin + InLocalRayDirection * distanceToWCapNeg;
			const FVector4 capRadial = FVector4(capPoint.X, capPoint.Y, 0.0, 0.0);
			if (capRadial.SizeSquared() <= radiusSq && FMath::Abs(capPoint.Z) <= halfHeight)
			{
				bestDistance = distanceToWCapNeg;
				bestNormal = FVector4(0, 0, 0, -1);
				result.bHit = true;
				result.Distance = distanceToWCapNeg;
				result.Normal = bestNormal;
				result.Point = capPoint;
			}
		}
	}
	// Check curved surface entrance (only if no cap hit or curved surface is closer).
	const double distanceToEntrance = (-radialPointDotRadialDir - sqrtDiscriminant) / radialDirLenSq;
	if (distanceToEntrance >= 0.0 && distanceToEntrance < bestDistance)
	{
		const FVector4 entrancePoint = InLocalRayOrigin + InLocalRayDirection * distanceToEntrance;
		if (FMath::Abs(entrancePoint.Z) <= halfHeight && FMath::Abs(entrancePoint.W) <= halfThickness)
		{
			// Entrance through curved surface.
			const FVector4 hitRadial = FVector4(entrancePoint.X, entrancePoint.Y, 0.0, 0.0);
			const FVector4 normal = hitRadial.GetSafeNormal();
			result.bHit = true;
			result.Distance = distanceToEntrance;
			result.Normal = normal;
			result.Point = entrancePoint;
			return result;
		}
	}
	return result;
}

double UCubinderShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const double halfHeight = Height * 0.5;
	const double halfThickness = Thickness * 0.5;
	FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, 0.0);
	const double lengthSq = nearest.SizeSquared();
	const double radialSignedDistance = FMath::Sqrt(lengthSq) - Radius;
	const double verticalSignedDistance = FMath::Abs(InLocalPoint.Z) - halfHeight;
	const double thicknessSignedDistance = FMath::Abs(InLocalPoint.W) - halfThickness;
	const double absRadialSignedDistance = FMath::Abs(radialSignedDistance);
	const double absVerticalSignedDistance = FMath::Abs(verticalSignedDistance);
	const double absThicknessSignedDistance = FMath::Abs(thicknessSignedDistance);
	if (OutNearestPointOnSurface != nullptr)
	{
		if (radialSignedDistance > 0.0 || absRadialSignedDistance >= absVerticalSignedDistance || absRadialSignedDistance >= absThicknessSignedDistance)
		{
			if (lengthSq == 0.0)
			{
				nearest = FVector4(Radius, 0.0, 0.0, 0.0); // Arbitrary point on the surface.
			}
			else
			{
				nearest *= Radius / FMath::Sqrt(lengthSq);
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
		if (thicknessSignedDistance > 0.0 || absThicknessSignedDistance > absRadialSignedDistance)
		{
			nearest.W = (InLocalPoint.W > 0.0) ? halfThickness : -halfThickness;
		}
		else
		{
			nearest.W = InLocalPoint.W;
		}
		*OutNearestPointOnSurface = nearest;
	}
	// Return the smallest signed distance, which corresponds to the closest surface.
	if (absThicknessSignedDistance < absRadialSignedDistance && absThicknessSignedDistance < absVerticalSignedDistance)
	{
		return thicknessSignedDistance;
	}
	if (absVerticalSignedDistance < absRadialSignedDistance)
	{
		return verticalSignedDistance;
	}
	return radialSignedDistance;
}

FVector4 UCubinderShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	// First, check the horizontal plane, which in Unreal means X and Y.
	FVector4 nearest = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, 0.0);
	const double lengthSq = nearest.SizeSquared();
	if (lengthSq > Radius * Radius)
	{
		nearest = nearest * Radius / FMath::Sqrt(lengthSq);
	}
	// Then clamp the vertical (Z) and thickness (W) axes.
	const double halfHeight = Height * 0.5;
	const double halfThickness = Thickness * 0.5;
	nearest.Z = FMath::Clamp(InLocalPoint.Z, -halfHeight, halfHeight);
	nearest.W = FMath::Clamp(InLocalPoint.W, -halfThickness, halfThickness);
	return nearest;
}

FVector4 UCubinderShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const double halfHeight = Height * 0.5;
	const double halfThickness = Thickness * 0.5;
	FVector4 support = FVector4(InLocalDirection.X, InLocalDirection.Y, 0.0, 0.0).GetSafeNormal() * Radius;
	support.Z = (InLocalDirection.Z > 0.0) ? halfHeight : -halfHeight;
	support.W = (InLocalDirection.W > 0.0) ? halfThickness : -halfThickness;
	return support;
}

bool UCubinderShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	FVector4 absPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	if (absPoint.Z > Height * 0.5)
	{
		return false;
	}
	absPoint.Z = 0.0;
	if (absPoint.W > Thickness * 0.5)
	{
		return false;
	}
	absPoint.W = 0.0;
	return absPoint.SizeSquared() <= Radius * Radius;
}

bool UCubinderShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UCubinderShape4DData* cubinder = Cast<UCubinderShape4DData>(InShape);
	if (cubinder == nullptr)
	{
		return false;
	}
	return Height == cubinder->Height && Radius == cubinder->Radius && Thickness == cubinder->Thickness;
}
