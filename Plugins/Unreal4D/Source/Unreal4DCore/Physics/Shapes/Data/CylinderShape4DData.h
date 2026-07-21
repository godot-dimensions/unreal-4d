#pragma once

#include "Shape4DData.h"

#include "CylinderShape4DData.generated.h"

// UCylinderShape4DData describes a 4D cylinder, also known as a spherinder,
// using a Height and Radius. The spherinder shape is like a 4D capsule,
// but with the end "hemispheres" removed. The cylinder is centered at the origin.
// The height spans vertically along the Z axis; if you want a cylinder facing
// a different direction, you can rotate the component the cylinder is attached to.
//
// When a 4D cylinder is viewed directly from the top or bottom, it appears as a
// flat 3D sphere, and when viewed from the side, it appears to be a cuboid.
// This is similar to how a 3D cylinder appears as a 2D circle when viewed from
// the top or bottom, and as a rectangle when viewed from the side.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCylinderShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Height = 200.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
