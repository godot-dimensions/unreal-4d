#pragma once

#include "Shape4DComponent.h"

#include "RayShape4DComponent.generated.h"

// URayShape4DComponent describes a 4D ray physics shape component.
// It defines a ray with a length that always points in the local -Z direction (down).
// The ray is unidirectional, meaning it only detects objects in the direction of the ray, not in the opposite direction.
// To change the position or rotation of the ray, move or rotate the component's 4D transform.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API URayShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RayTargetZ = -100.0; // Note: This MUST NOT be positive.

	double GetRayLength() const { return -RayTargetZ; }
	void SetRayLength(const double InLength);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
