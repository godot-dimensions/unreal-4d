#pragma once

#include "Shape4DComponent.h"

#include "CubinderShape4DComponent.generated.h"

// UCubinderShape4DComponent is a shape created by extruding a 3D cylinder into 4D space.
// It is defined by a radius of a circle in the XZ plane, a height in the Y direction,
// and a thickness in the W direction. The cubinder is centered at the origin.
// If you want a cubinder in a different orientation, you can rotate the component's 4D transform.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UCubinderShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Height = 200.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Radius = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double Thickness = 200.0;

	void SetHeight(const double InNewHeight);
	void SetRadius(const double InNewRadius);
	void SetThickness(const double InNewThickness);
	FVector2D GetSizeZW() const;
	void SetSizeZW(const FVector2D& InSizeZW);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
