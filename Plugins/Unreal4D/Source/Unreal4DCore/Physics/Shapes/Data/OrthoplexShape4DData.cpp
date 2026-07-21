#include "OrthoplexShape4DData.h"

#include "../../../Math/Plane4D.h"
#include "../../../Math/VectorFuncs4D.h"
#include "../../../Model/Mesh/Tetra/OrthoplexTetraMesh4D.h"

FVector4 UOrthoplexShape4DData::GetHalfExtents() const
{
	return Size * 0.5;
}

void UOrthoplexShape4DData::SetHalfExtents(const FVector4& InHalfExtents)
{
	Size = InHalfExtents * 2.0;
}

double UOrthoplexShape4DData::GetHypervolume() const
{
	// http://hi.gher.space/wiki/16-cell
	// Bulk of regular orthoplex is (1/6) * edge_length ^ 4, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 4 times, so 1/4, so (1/6)*(1/4) becomes 1/24.
	return (1.0 / 24.0) * Size.X * Size.Y * Size.Z * Size.W;
}

double UOrthoplexShape4DData::GetSurfaceVolume() const
{
	// Surcell volume of regular orthoplex is (4*sqrt(2)/3) * edge_length ^ 3, but we have size instead of edge length.
	// To convert we need to divide by sqrt(2) 3 times, so 1/(2*sqrt(2)), the sqrt(2)s cancel out, so (4/3)*(1/2) = 2/3.
	// Then since we have each axis separate, we need to multiply by a quarter 4 times, so (2/3)*(1/4) = 1/6.
	return (1.0 / 6.0) * (Size.X * Size.Y * Size.Z + Size.X * Size.Y * Size.W + Size.X * Size.Z * Size.W + Size.Y * Size.Z * Size.W);
}

FRect4D UOrthoplexShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	FRect4D bounds = FRect4D(InTransformToTarget.Origin, FVector4(0.0, 0.0, 0.0, 0.0));
	const FVector4 halfExtents = GetHalfExtents();
	for (int32 i = 0; i < 4; i++)
	{
		for (int32 j = 0; j < 2; j++)
		{
			FVector4 point = FVector4(0.0, 0.0, 0.0, 0.0);
			point[i] = j == 0 ? -halfExtents[i] : halfExtents[i];
			bounds = bounds.ExpandToPoint(InTransformToTarget * point);
		}
	}
	return bounds;
}

FRaycastResult4D UOrthoplexShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	FRaycastResult4D result;
	if (!FMath::IsNearlyEqual(InLocalRayDirection.SizeSquared(), 1.0))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UOrthoplexShape4DData::RaycastIntersectsLocal4D: Ray direction must be normalized."));
		return result;
	}
	if (bInInsideIsZero && HasLocalPoint(InLocalRayOrigin))
	{
		result.bHit = true;
		result.Point = InLocalRayOrigin;
		return result;
	}
	// Convert the raycast vectors into a space that assumes the orthoplex is of unit size.
	const FVector4 rayFrom = InLocalRayOrigin / Size;
	const FVector4 rayDirection = (InLocalRayDirection / Size).GetSafeNormal();
	FVector4 bestNormal = FVector4(0.0, 0.0, 0.0, 0.0);
	double bestDistance = std::numeric_limits<double>::infinity();
	// Iterate over the 16 planes of the orthoplex.
	for (double x = -0.5; x <= 0.5; x += 1.0)
	{
		for (double y = -0.5; y <= 0.5; y += 1.0)
		{
			for (double z = -0.5; z <= 0.5; z += 1.0)
			{
				for (double w = -0.5; w <= 0.5; w += 1.0)
				{
					const FHyperplane4D plane = FHyperplane4D(FVector4(x, y, z, w), 0.5);
					const double factor = plane.IntersectRayFactor(rayFrom, rayDirection);
					if (factor >= 0.0 && factor < bestDistance)
					{
						const FVector4 hitPointAbs = UVectorFuncs4D::Vector4_Abs(rayFrom + rayDirection * factor);
						// Similar to HasPoint but does not scale by the size and has a slightly larger tolerance.
						if ((hitPointAbs.X + hitPointAbs.Y + hitPointAbs.Z + hitPointAbs.W) <= 1.000001)
						{
							bestDistance = factor;
							bestNormal = plane.Normal;
						}
					}
				}
			}
		}
	}
	// The distance above is in the scaled space, so we need to return it to the original space and calculate a new distance.
	const FVector4 hitPointAccountingForSize = (rayFrom + rayDirection * bestDistance) * Size;
	bestDistance = UVectorFuncs4D::Vector4_Distance(InLocalRayOrigin, hitPointAccountingForSize);
	const bool hit = bestDistance < InMaxDistance;
	result.bHit = hit;
	if (hit)
	{
		result.Point = hitPointAccountingForSize;
		result.Distance = bestDistance;
		// Divide by the size again, aka multiply by the inverse of the size.
		// Ex: Larger size on X means the "faces" point more in the YZW directions, so the normal is smaller in X.
		result.Normal = (bestNormal / Size).GetSafeNormal();
	}
	return result;
}

double UOrthoplexShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	if (InLocalPoint == FVector4(0.0, 0.0, 0.0, 0.0))
	{
		if (OutNearestPointOnSurface != nullptr)
		{
			*OutNearestPointOnSurface = FVector4(0.5 * Size.X, 0.0, 0.0, 0.0);
		}
		return -0.5 * Size.X;
	}
	const FVector4 absScaledPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint) / Size;
	const double scaledSignedDistance = (absScaledPoint.X + absScaledPoint.Y + absScaledPoint.Z + absScaledPoint.W) - 1.0;
	const double adjustPerAxis = scaledSignedDistance * -0.25;
	FVector4 nearestPoint = absScaledPoint + FVector4(adjustPerAxis, adjustPerAxis, adjustPerAxis, adjustPerAxis);
	nearestPoint *= (UVectorFuncs4D::Vector4_Sign(InLocalPoint) * Size);
	if (OutNearestPointOnSurface != nullptr)
	{
		*OutNearestPointOnSurface = nearestPoint;
	}
	const double distance = UVectorFuncs4D::Vector4_Distance(InLocalPoint, nearestPoint);
	return (scaledSignedDistance < 0.0) ? -distance : distance;
}

FVector4 UOrthoplexShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	return UVectorFuncs4D::Vector4_LimitLengthTaxicab(InLocalPoint / Size, 0.5) * Size;
}

FVector4 UOrthoplexShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const FVector4 absDir = UVectorFuncs4D::Vector4_Abs(InLocalDirection);
	uint8 longestAxis = 0;
	double maxValue = absDir[0];
	for (int32 i = 1; i < 4; i++)
	{
		if (absDir[i] > maxValue)
		{
			maxValue = absDir[i];
			longestAxis = i;
		}
	}
	FVector4 support = FVector4(0.0, 0.0, 0.0, 0.0);
	support[longestAxis] = (InLocalDirection[longestAxis] > 0.0) ? Size[longestAxis] * 0.5 : -Size[longestAxis] * 0.5;
	return support;
}

bool UOrthoplexShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	const FVector4 absScaledPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint) / Size;
	return (absScaledPoint.X + absScaledPoint.Y + absScaledPoint.Z + absScaledPoint.W) <= 1.0;
}

bool UOrthoplexShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UOrthoplexShape4DData* otherShape = Cast<UOrthoplexShape4DData>(InShape);
	if (otherShape == nullptr)
	{
		return false;
	}
	return Size == otherShape->Size;
}

UTetraMesh4D* UOrthoplexShape4DData::ToTetraMesh() const
{
	UOrthoplexTetraMesh4D* tetraMesh = NewObject<UOrthoplexTetraMesh4D>();
	tetraMesh->SetSize(Size);
	return tetraMesh;
}
