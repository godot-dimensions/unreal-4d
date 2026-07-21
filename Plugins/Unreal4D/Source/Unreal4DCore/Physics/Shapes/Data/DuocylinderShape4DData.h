#pragma once

#include "Shape4DData.h"

#include "DuocylinderShape4DData.generated.h"

// UDuocylinderShape4DData describes a 4D duocylinder, the cartesian product
// of two circles in 4D space. It is defined by the radiuses of two circles, one in
// the XY plane and one in the ZW plane. The duocylinder is centered at the origin.
// If you want a duocylinder in a different orientation, you can rotate the component the duocylinder is attached to.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UDuocylinderShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RadiusXY = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RadiusZW = 50.0;

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
