#pragma once

#include "Shape4DComponent.h"

#include "DuocylinderShape4DComponent.generated.h"

// UDuocylinderShape4DComponent describes a 4D duocylinder, the cartesian product
// of two circles in 4D space. It is defined by the radiuses of two circles, one in
// the XY plane and one in the ZW plane. The duocylinder is centered at the origin.
// If you want a duocylinder in a different orientation, you can rotate the component's 4D transform.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UDuocylinderShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RadiusXY = 50.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	double RadiusZW = 50.0;

	void SetRadiusXY(const double InNewRadiusXY);
	void SetRadiusZW(const double InNewRadiusZW);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
