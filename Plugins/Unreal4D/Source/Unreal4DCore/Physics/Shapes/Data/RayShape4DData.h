#pragma once

#include "Shape4DData.h"

#include "RayShape4DData.generated.h"

// URayShape4DData describes a 4D ray physics shape data.
// It defines a ray with a length that always points in the local -Z direction (down).
// The ray is unidirectional, meaning it only detects objects in the direction of the ray, not in the opposite direction.
// To change the position or rotation of the ray, move or rotate the component the ray is attached to.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API URayShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RayTargetZ = -100.0; // Note: This MUST NOT be positive.

	double GetRayLength() const { return -RayTargetZ; }
	void SetRayLength(double InLength) { RayTargetZ = -InLength; }

	virtual double GetHypervolume() const override { return 0.0; }
	virtual double GetSurfaceVolume() const override { return 0.0; }
	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget = FTransform4D()) const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
