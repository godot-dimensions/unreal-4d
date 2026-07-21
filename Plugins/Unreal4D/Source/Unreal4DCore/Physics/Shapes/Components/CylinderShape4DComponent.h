#pragma once

#include "Shape4DComponent.h"

#include "CylinderShape4DComponent.generated.h"

// UCylinderShape4DComponent describes a 4D cylinder, also known as a spherinder,
// using a Height and Radius. The spherinder shape is like a 4D capsule,
// but with the end "hemispheres" removed. The cylinder is centered at the origin.
// The height spans vertically along the Z axis; if you want a cylinder facing
// a different direction, you can rotate the component's 4D transform.
//
// When a 4D cylinder is viewed directly from the top or bottom, it appears as a
// flat 3D sphere, and when viewed from the side, it appears to be a cuboid.
// This is similar to how a 3D cylinder appears as a 2D circle when viewed from
// the top or bottom, and as a rectangle when viewed from the side.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCylinderShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Height = 200.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;

	void SetHeight(const double InNewHeight);
	void SetRadius(const double InNewRadius);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
