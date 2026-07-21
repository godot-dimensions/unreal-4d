#pragma once

#include "Shape4DData.h"

#include "CapsuleShape4DData.generated.h"

// UCapsuleShape4DData describes a 4D capsule using a Height and Radius.
// The capsule is centered at the origin. The height spans vertically along the Z axis;
// if you want a capsule facing a different direction, you can rotate the data's 4D transform.
//
// When a 4D capsule is viewed directly from the top or bottom, it appears
// as a 3D sphere, with the center of the sphere closer to the viewer.
// This is similar to how a 3D capsule appears as a 2D circle when viewed from
// the top or bottom, with the center of the circle closer to the viewer.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCapsuleShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Height = 200.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	// If true, Height is the full height of the capsule. If false, Height is the mid-height of the capsule.
	bool bHeightIsFull = true;

	double GetFullHeight() const;
	void SetFullHeight(const double InFullHeight);
	double GetMidHeight() const;
	void SetMidHeight(const double InMidHeight);
	void SetHeightIsFull(const bool bInHeightIsFull);

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
