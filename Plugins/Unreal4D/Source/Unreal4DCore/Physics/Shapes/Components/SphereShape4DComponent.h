#pragma once

#include "Shape4DComponent.h"

#include "SphereShape4DComponent.generated.h"

// USphereShape4DComponent describes a 4D (hyper)sphere, or glome, using a radius.
// The sphere is centered at the origin. Spheres are symmetric in
// all directions, so rotation does not affect the shape.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API USphereShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;

	void SetRadius(const double InNewRadius);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Physics|Shape")
	double GetRadiusWorld() const;

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
