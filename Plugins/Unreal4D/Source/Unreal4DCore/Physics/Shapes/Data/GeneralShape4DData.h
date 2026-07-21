#pragma once

#include "Shape4DData.h"
#include "GeneralShapeCurve4D.h"

#include "GeneralShape4DData.generated.h"

// UGeneralShape4DData describes a highly generalized 4D physics shape.
// It can be used to create a wide variety of shapes, but should be avoided if other shapes
// can be used instead, as it is much less efficient to calculate collisions with.
//
// The base of a general shape is a box defined by the BaseSize property, which is the size of a
// 4D box in all directions as an FVector4. This alone can be used to create a UBoxShape4DData.
// This includes 4D curves used on hyperspheres like USphereShape4DData and UCapsuleShape4DData,
// 3D curves used on shapes like UCylinderShape4DData, and 2D curves used on shapes like
// UCubinderShape4DData and UDuocylinderShape4DData.
//
// The difference between a UGeneralShape4DData set up like a USphereShape4DData
// versus a UCapsuleShape4DData is that the former has a BaseSize of zero,
// while the latter has a BaseSize with the Y data set to the mid-height of the capsule.
// Both have a single 4D curve, which creates a "swept sphere" shape over the base size.
// This also means that a BaseSize with two values set to non-zero creates what looks like a rounded rectangle
// from the correct angle, and a BaseSize with all four values set to non-zero creates a rounded 4D box.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UGeneralShape4DData : public UShape4DData
{
	GENERATED_BODY()

	static bool _warningsEnabled;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 BaseSize = FVector4(0.0, 0.0, 0.0, 0.0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	TArray<TObjectPtr<UGeneralShapeCurve4D>> Curves;

	FVector4 GetBaseHalfExtents() const { return BaseSize * 0.5; }
	void SetBaseSize(const FVector4& InNewBaseSize) { BaseSize = InNewBaseSize; }
	void SetBaseHalfExtents(const FVector4& InBaseHalfExtents) { BaseSize = InBaseHalfExtents * 2.0; }

	static void SetWarningsEnabled(bool InWarningsEnabled) { _warningsEnabled = InWarningsEnabled; }

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;
	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget = FTransform4D()) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;
};
