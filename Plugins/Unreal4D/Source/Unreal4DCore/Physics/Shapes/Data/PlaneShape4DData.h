#pragma once

#include "Shape4DData.h"

#include "PlaneShape4DData.generated.h"

// UPlaneShape4DData describes a 4D plane physics shape data with a 3D surface.
// It can be used to create world boundaries, such as an infinite floor or infinite walls.
// Planes may only be used with non-moving bodies, and cannot be used with moving bodies.
//
// For simplicity, the plane always has its normal vector pointing in the local +Z direction,
// and always intersects the local origin. This means the plane has no properties.
// The plane is infinite in the local X, Y, and W directions.
// To make a finite plane instead of an infinite one, use a UBoxShape4DData instead.
// To change the position or rotation of the plane, move or rotate the component the plane is attached to.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UPlaneShape4DData : public UShape4DData
{
	GENERATED_BODY()

	// The plane is supposed to be infinite, but for some calculations, provide a finite size.
	static constexpr double PLANE_WIDTH = 1e18;
	static constexpr double PLANE_START = PLANE_WIDTH * -0.5;
	static constexpr double PLANE_THICKNESS = 1e3;

public:
	virtual double GetHypervolume() const override { return std::numeric_limits<double>::infinity(); }
	virtual double GetSurfaceVolume() const override { return std::numeric_limits<double>::infinity(); }
	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget = FTransform4D()) const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
