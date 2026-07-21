#pragma once

#include "Shape4DData.h"

#include "SphereShape4DData.generated.h"

// USphereShape4DData describes a 4D (hyper)sphere using a radius.
// The sphere is centered at the origin. Spheres are symmetric in
// all directions, so rotation does not affect the shape.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API USphereShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
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
