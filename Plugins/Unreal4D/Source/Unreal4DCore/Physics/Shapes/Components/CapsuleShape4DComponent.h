#pragma once

#include "Shape4DComponent.h"

#include "CapsuleShape4DComponent.generated.h"

// UCapsuleShape4DComponent describes a 4D capsule using a Height and Radius.
// The capsule is centered at the origin. The height spans vertically along the Z axis;
// if you want a capsule facing a different direction, you can rotate the component's 4D transform.
//
// When a 4D capsule is viewed directly from the top or bottom, it appears
// as a 3D sphere, with the center of the sphere closer to the viewer.
// This is similar to how a 3D capsule appears as a 2D circle when viewed from
// the top or bottom, with the center of the circle closer to the viewer.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCapsuleShape4DComponent : public UShape4DComponent
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

	void SetRadius(const double InNewRadius);
	double GetFullHeight() const;
	void SetFullHeight(const double InFullHeight);
	double GetMidHeight() const;
	void SetMidHeight(const double InMidHeight);
	void SetHeightIsFull(const bool bInHeightIsFull);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
