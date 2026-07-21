#pragma once

#include "../../../Unreal4DDefines.h"

#include "GeneralShapeCurve4D.generated.h"

// FGeneralShapeCurveTaperPoint4D is a minimal struct that defines a taper point
// for a curve on a general shape. See UGeneralShapeCurve4D for more details.
USTRUCT(Category = "Unreal 4D|Physics|Shape")
struct FGeneralShapeCurveTaperPoint4D
{
	GENERATED_USTRUCT_BODY()

public:
	FVector4 Position = FVector4(0.0, 0.0, 0.0, 0.0);
	FVector4 Radii = FVector4(0.0, 0.0, 0.0, 0.0); // MUST NOT be negative.
	double Exponent = 2.0; // MUST be positive and finite.

	bool IsEqualExact(const FGeneralShapeCurveTaperPoint4D& InOther) const;
};

// UGeneralShapeCurve4D describes a curve inside of a UGeneralShape4DData,
// which is a 4D physics shape data. It can describe a wide variety of curves,
// including 4D curves used on hyperspheres like USphereShape4DData and
// UCapsuleShape4DData, 3D curves used on shapes like UCylinderShape4DData,
// and 2D curves used on shapes like UCubinderShape4DData and
// UDuocylinderShape4DData. The curves may be used on any axes by setting
// the Radii property, and any axes with zero Radii are not used for the curve.
//
// Additionally, curves may by ellipsoidal by setting different radius values, have a custom
// exponent to make the shape pointier or rounder, and may have tapering defined, however these
// features are not yet supported in most calculations. In many cases, closed form solutions do
// not exist. It is recommended to not use ellipsoidal curves, custom exponents, or tapering.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UGeneralShapeCurve4D : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 Radii = FVector4(0.0, 0.0, 0.0, 0.0); // MUST NOT be negative.
	UPROPERTY(EditAnywhere, Category = "Unreal 4D|Physics|Shape")
	TArray<FGeneralShapeCurveTaperPoint4D> Taper;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Exponent = 2.0; // MUST be positive and finite.

	int32 GetRadiiDimension() const;
	double GetRadiiSum() const;
	int32 GetTaperCount() const { return Taper.Num(); }
	TArray<int32> GetUsedAxes() const;
	TArray<int32> GetZeroAxes() const;
	bool IsEqualExact(const UGeneralShapeCurve4D* InOther) const;
};
