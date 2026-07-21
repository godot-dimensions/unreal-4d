#include "DuocylinderShape4DData.h"

double UDuocylinderShape4DData::GetHypervolume() const
{
	// http://hi.gher.space/wiki/Duocylinder
	return (UE_DOUBLE_PI * UE_DOUBLE_PI) * RadiusXY * RadiusXY * RadiusZW * RadiusZW;
}

FRaycastResult4D UDuocylinderShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UDuocylinderShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	if (bInInsideIsZero && HasLocalPoint(InLocalRayOrigin))
	{
		result.bHit = true;
		result.Point = InLocalRayOrigin;
		return result;
	}
	// Perform two ray-circle intersection tests, one for the XY circle and one for the ZW circle.
	FVector4 bestNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistance = InMaxDistance;
	// Check XY circle intersection.
	const FVector2D xyPoint = FVector2D(InLocalRayOrigin.X, InLocalRayOrigin.Y);
	const FVector2D xyDir = FVector2D(InLocalRayDirection.X, InLocalRayDirection.Y);
	const double xyDirLenSq = xyDir.SizeSquared();
	if (xyDirLenSq < UNREAL4D_COMPARISON_EPSILON)
	{
		// Ray is parallel to XY plane, check if the starting point is inside the XY circle.
		if (xyPoint.SizeSquared() >= RadiusXY * RadiusXY)
		{
			return result; // No intersection, ray is outside the XY circle.
		}
	}
	else
	{
		// Ray moves along the XY plane, perform ray-circle intersection test.
		// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
		const double xyPointDotXyDir = xyPoint.Dot(xyDir);
		const double xyPointDotXyPoint = xyPoint.Dot(xyPoint);
		const double discriminantXy = xyPointDotXyDir * xyPointDotXyDir - xyDirLenSq * (xyPointDotXyPoint - RadiusXY * RadiusXY);
		if (discriminantXy >= 0.0)
		{
			const double sqrtDiscriminantXy = FMath::Sqrt(discriminantXy);
			const double xyDistanceNear = (-xyPointDotXyDir - sqrtDiscriminantXy) / xyDirLenSq;
			const double xyDistanceFar = (-xyPointDotXyDir + sqrtDiscriminantXy) / xyDirLenSq;
			const double xyDistance = xyDistanceNear >= 0.0 ? xyDistanceNear : xyDistanceFar;
			if (xyDistance >= 0.0)
			{
				const FVector4 hitPoint = InLocalRayOrigin + InLocalRayDirection * xyDistance;
				const FVector2D zwCheck = FVector2D(hitPoint.Z, hitPoint.W);
				if (zwCheck.SizeSquared() <= RadiusZW * RadiusZW)
				{
					if (xyDistance < bestDistance)
					{
						bestDistance = xyDistance;
						const FVector2D xyHit = FVector2D(hitPoint.X, hitPoint.Y);
						const FVector2D xyNormalized = xyHit.GetSafeNormal();
						bestNormal = FVector4(xyNormalized.X, xyNormalized.Y, 0.0, 0.0);
					}
				}
			}
		}
	}
	// Check ZW circle intersection.
	const FVector2D zwPoint = FVector2D(InLocalRayOrigin.Z, InLocalRayOrigin.W);
	const FVector2D zwDir = FVector2D(InLocalRayDirection.Z, InLocalRayDirection.W);
	const double zwDirLenSq = zwDir.SizeSquared();
	if (zwDirLenSq < UNREAL4D_COMPARISON_EPSILON)
	{
		// Ray is parallel to ZW plane, check if the starting point is inside the ZW circle.
		if (zwPoint.SizeSquared() >= RadiusZW * RadiusZW)
		{
			return result; // No intersection, ray is outside the ZW circle.
		}
	}
	else
	{
		// Ray moves along the ZW plane, perform ray-circle intersection test.
		// Disclaimer: This code was mostly AI-generated. I am not sure if it is correct, but it works in testing.
		const double zwPointDotZwDir = zwPoint.Dot(zwDir);
		const double zwPointDotZwPoint = zwPoint.Dot(zwPoint);
		const double discriminantZw = zwPointDotZwDir * zwPointDotZwDir - zwDirLenSq * (zwPointDotZwPoint - RadiusZW * RadiusZW);
		if (discriminantZw >= 0.0)
		{
			const double sqrtDiscriminantZw = FMath::Sqrt(discriminantZw);
			const double zwDistanceNear = (-zwPointDotZwDir - sqrtDiscriminantZw) / zwDirLenSq;
			const double zwDistanceFar = (-zwPointDotZwDir + sqrtDiscriminantZw) / zwDirLenSq;
			const double zwDistance = zwDistanceNear >= 0.0 ? zwDistanceNear : zwDistanceFar;
			if (zwDistance >= 0.0 && zwDistance < bestDistance)
			{
				const FVector4 hitPoint = InLocalRayOrigin + InLocalRayDirection * zwDistance;
				const FVector2D xyCheck = FVector2D(hitPoint.X, hitPoint.Y);
				if (xyCheck.SizeSquared() <= RadiusXY * RadiusXY)
				{
					bestDistance = zwDistance;
					const FVector2D zwHit = FVector2D(hitPoint.Z, hitPoint.W);
					const FVector2D zwNormalized = zwHit.GetSafeNormal();
					bestNormal = FVector4(0.0, 0.0, zwNormalized.X, zwNormalized.Y);
				}
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

double UDuocylinderShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const FVector4 xyPoint = FVector4(InLocalPoint.X, InLocalPoint.Y, 0.0, 0.0);
	const FVector4 zwPoint = FVector4(0.0, 0.0, InLocalPoint.Z, InLocalPoint.W);
	const double xyLenSq = xyPoint.SizeSquared();
	const double zwLenSq = zwPoint.SizeSquared();
	if (xyLenSq < zwLenSq)
	{
		const double xyLen = FMath::Sqrt(xyLenSq);
		const double xySignedDistance = xyLen - RadiusXY;
		if (OutNearestPointOnSurface != nullptr)
		{
			if (xyLen == 0.0)
			{
				*OutNearestPointOnSurface = FVector4(RadiusXY, 0.0, InLocalPoint.Z, InLocalPoint.W); // Arbitrary point on the surface.
			}
			else
			{
				const double xyAdjust = RadiusXY / xyLen;
				*OutNearestPointOnSurface = FVector4(xyPoint.X * xyAdjust, xyPoint.Y * xyAdjust, InLocalPoint.Z, InLocalPoint.W);
			}
		}
		return xySignedDistance;
	}
	const double zwLen = FMath::Sqrt(zwLenSq);
	const double zwSignedDistance = zwLen - RadiusZW;
	if (OutNearestPointOnSurface != nullptr)
	{
		if (zwLen == 0.0)
		{
			*OutNearestPointOnSurface = FVector4(InLocalPoint.X, InLocalPoint.Y, RadiusZW, 0.0); // Arbitrary point on the surface.
		}
		else
		{
			const double zwAdjust = RadiusZW / zwLen;
			*OutNearestPointOnSurface = FVector4(InLocalPoint.X, InLocalPoint.Y, zwPoint.Z * zwAdjust, zwPoint.W * zwAdjust);
		}
	}
	return zwSignedDistance;
}

FVector4 UDuocylinderShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	FVector2D xy = FVector2D(InLocalPoint.X, InLocalPoint.Y);
	const double xyLenSq = xy.SizeSquared();
	if (xyLenSq > RadiusXY * RadiusXY)
	{
		xy *= RadiusXY / FMath::Sqrt(xyLenSq);
	}
	FVector2D zw = FVector2D(InLocalPoint.Z, InLocalPoint.W);
	const double zwLenSq = zw.SizeSquared();
	if (zwLenSq > RadiusZW * RadiusZW)
	{
		zw *= RadiusZW / FMath::Sqrt(zwLenSq);
	}
	return FVector4(xy.X, xy.Y, zw.X, zw.Y);
}

FVector4 UDuocylinderShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const FVector2D supportXY = FVector2D(InLocalDirection.X, InLocalDirection.Y).GetSafeNormal() * RadiusXY;
	const FVector2D supportZW = FVector2D(InLocalDirection.Z, InLocalDirection.W).GetSafeNormal() * RadiusZW;
	return FVector4(supportXY.X, supportXY.Y, supportZW.X, supportZW.Y);
}

double UDuocylinderShape4DData::GetSurfaceVolume() const
{
	return (2.0 * UE_DOUBLE_PI * UE_DOUBLE_PI) * RadiusXY * RadiusZW * (RadiusXY + RadiusZW);
}

bool UDuocylinderShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	const bool inXY = InLocalPoint.X * InLocalPoint.X + InLocalPoint.Y * InLocalPoint.Y <= RadiusXY * RadiusXY;
	const bool inZW = InLocalPoint.Z * InLocalPoint.Z + InLocalPoint.W * InLocalPoint.W <= RadiusZW * RadiusZW;
	return inXY && inZW;
}

bool UDuocylinderShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UDuocylinderShape4DData* shape = Cast<UDuocylinderShape4DData>(InShape);
	if (shape == nullptr)
	{
		return false;
	}
	return RadiusXY == shape->RadiusXY && RadiusZW == shape->RadiusZW;
}
