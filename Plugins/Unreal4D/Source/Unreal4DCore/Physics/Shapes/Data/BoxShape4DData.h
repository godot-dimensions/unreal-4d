#pragma once

#include "Shape4DData.h"

#include "BoxShape4DData.generated.h"

// UBoxShape4DData describes a 4D box using a Size as an FVector4.
// The box is centered at the origin, and is axis-aligned by default;
// if you want a box facing a different direction, you can rotate the
// data that is using the shape, such as UBoxShape4DData.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UBoxShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

	FVector4 GetHalfExtents() const;
	void SetHalfExtents(const FVector4& InHalfExtents);

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;

	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget = FTransform4D()) const override;
	FRect4D GetRectBoundsFast4D() const;
	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;

	virtual UTetraMesh4D* ToTetraMesh() const override;
};
