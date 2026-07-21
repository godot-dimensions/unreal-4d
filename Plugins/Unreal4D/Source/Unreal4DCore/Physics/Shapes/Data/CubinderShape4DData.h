#pragma once

#include "Shape4DData.h"

#include "CubinderShape4DData.generated.h"

// UCubinderShape4DData is a shape created by extruding a 3D cylinder into 4D space.
// It is defined by a radius of a circle in the XZ plane, a height in the Y direction,
// and a thickness in the W direction. The cubinder is centered at the origin.
// If you want a cubinder in a different orientation, you can rotate the component the cubinder is attached to.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCubinderShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Height = 200.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Thickness = 200.0;

	FVector2D GetSizeZW() const;
	void SetSizeZW(const FVector2D& InSizeZW);

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
