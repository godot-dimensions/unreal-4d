#pragma once

#include "Shape4DComponent.h"

#include "PlaneShape4DComponent.generated.h"

// UPlaneShape4DComponent describes a 4D plane physics shape component with a 3D surface.
// It can be used to create world boundaries, such as an infinite floor or infinite walls.
// Planes may only be used with non-moving bodies, and cannot be used with moving bodies.
//
// For simplicity, the plane always has its normal vector pointing in the local +Z direction,
// and always intersects the local origin. This means the plane has no properties.
// The plane is infinite in the local X, Y, and W directions.
// To make a finite plane instead of an infinite one, use a UBoxShape4DComponent instead.
// To change the position or rotation of the plane, move or rotate the component's 4D transform.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UPlaneShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
