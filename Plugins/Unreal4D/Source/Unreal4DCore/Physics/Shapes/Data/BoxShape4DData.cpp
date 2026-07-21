#include "BoxShape4DData.h"

#include "../../../Model/Mesh/Tetra/BoxTetraMesh4D.h"

FVector4 UBoxShape4DData::GetHalfExtents() const
{
	return Size * 0.5;
}

void UBoxShape4DData::SetHalfExtents(const FVector4& InHalfExtents)
{
	Size = InHalfExtents * 2.0;
}

double UBoxShape4DData::GetHypervolume() const
{
	return Size.X * Size.Y * Size.Z * Size.W;
}

double UBoxShape4DData::GetSurfaceVolume() const
{
	return 2.0 * (Size.X * Size.Y * Size.Z + Size.X * Size.Y * Size.W + Size.X * Size.Z * Size.W + Size.Y * Size.Z * Size.W);
}

FRect4D UBoxShape4DData::GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget) const
{
	FRect4D bounds = FRect4D(InTransformToTarget.Origin, FVector4(0.0, 0.0, 0.0, 0.0));
	const FVector4 halfExtents = GetHalfExtents();
	for (int32 x = 0; x < 2; x++)
	{
		for (int32 y = 0; y < 2; y++)
		{
			for (int32 z = 0; z < 2; z++)
			{
				for (int32 w = 0; w < 2; w++)
				{
					const FVector4 point = FVector4(
							x == 0 ? -halfExtents.X : halfExtents.X,
							y == 0 ? -halfExtents.Y : halfExtents.Y,
							z == 0 ? -halfExtents.Z : halfExtents.Z,
							w == 0 ? -halfExtents.W : halfExtents.W);
					bounds = bounds.ExpandToPoint(InTransformToTarget * point);
				}
			}
		}
	}
	return bounds;
}

FRect4D UBoxShape4DData::GetRectBoundsFast4D() const
{
	const FVector4 halfExtents = GetHalfExtents();
	return FRect4D(-halfExtents, Size);
}

FRaycastResult4D UBoxShape4DData::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero) const
{
	const FRect4D localBounds = GetRectBoundsFast4D();
	if (bInInsideIsZero && localBounds.HasPoint(InLocalRayOrigin))
	{
		return FRaycastResult4D::EmptyWithHit();
	}
	// Pass false here because we handle the inside case above (avoid unnecessary calls and checks).
	return localBounds.RaycastIntersectsFull(InLocalRayOrigin, InLocalRayDirection, InMaxDistance, false);
}

double UBoxShape4DData::GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface) const
{
	const FVector4 halfExtents = GetHalfExtents();
	const FVector4 absPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	const FVector4 absToSurface = absPoint - halfExtents;
	const FVector4 absToSurfaceAbs = UVectorFuncs4D::Vector4_Abs(absToSurface);
	double nearestDistanceSigned = std::numeric_limits<double>::infinity();
	double nearestDistanceAbs = std::numeric_limits<double>::infinity();
	int8_t nearestAxis = 0;
	for (int8_t axis = 0; axis < 4; axis++)
	{
		if (absToSurfaceAbs[axis] < nearestDistanceAbs)
		{
			nearestDistanceSigned = absToSurface[axis];
			nearestDistanceAbs = absToSurfaceAbs[axis];
			nearestAxis = axis;
		}
	}
	if (OutNearestPointOnSurface != nullptr)
	{
		FVector4 nearestPoint = FVector4(
				FMath::Clamp(InLocalPoint.X, -halfExtents.X, halfExtents.X),
				FMath::Clamp(InLocalPoint.Y, -halfExtents.Y, halfExtents.Y),
				FMath::Clamp(InLocalPoint.Z, -halfExtents.Z, halfExtents.Z),
				FMath::Clamp(InLocalPoint.W, -halfExtents.W, halfExtents.W));
		nearestPoint[nearestAxis] = (InLocalPoint[nearestAxis] > 0.0) ? halfExtents[nearestAxis] : -halfExtents[nearestAxis];
		*OutNearestPointOnSurface = nearestPoint;
	}
	return nearestDistanceSigned;
}

FVector4 UBoxShape4DData::GetNearestPointToLocal(const FVector4& InLocalPoint) const
{
	const FVector4 halfExtents = GetHalfExtents();
	return FVector4(
			FMath::Clamp(InLocalPoint.X, -halfExtents.X, halfExtents.X),
			FMath::Clamp(InLocalPoint.Y, -halfExtents.Y, halfExtents.Y),
			FMath::Clamp(InLocalPoint.Z, -halfExtents.Z, halfExtents.Z),
			FMath::Clamp(InLocalPoint.W, -halfExtents.W, halfExtents.W));
}

FVector4 UBoxShape4DData::GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const
{
	const FVector4 halfExtents = GetHalfExtents();
	return FVector4(
			(InLocalDirection.X > 0.0) ? halfExtents.X : -halfExtents.X,
			(InLocalDirection.Y > 0.0) ? halfExtents.Y : -halfExtents.Y,
			(InLocalDirection.Z > 0.0) ? halfExtents.Z : -halfExtents.Z,
			(InLocalDirection.W > 0.0) ? halfExtents.W : -halfExtents.W);
}

bool UBoxShape4DData::HasLocalPoint(const FVector4& InLocalPoint) const
{
	const FVector4 absPoint = UVectorFuncs4D::Vector4_Abs(InLocalPoint);
	const FVector4 halfExtents = GetHalfExtents();
	return absPoint.X <= halfExtents.X && absPoint.Y <= halfExtents.Y && absPoint.Z <= halfExtents.Z && absPoint.W <= halfExtents.W;
}

bool UBoxShape4DData::IsEqualExact(const UShape4DData* InShape) const
{
	const UBoxShape4DData* boxShape = Cast<UBoxShape4DData>(InShape);
	if (boxShape == nullptr)
	{
		return false;
	}
	return Size == boxShape->Size;
}

UTetraMesh4D* UBoxShape4DData::ToTetraMesh() const
{
	UBoxTetraMesh4D* tetraMesh = NewObject<UBoxTetraMesh4D>();
	tetraMesh->SetSize(Size);
	return tetraMesh;
}
