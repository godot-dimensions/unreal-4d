#pragma once

#include "Shape4DComponent.h"

#include "BoxShape4DComponent.generated.h"

// UBoxShape4DComponent describes a 4D box using a Size as an FVector4.
// The box is centered at the origin, and is axis-aligned by default;
// if you want a box facing a different direction, you can rotate the component's 4D transform.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UBoxShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

	void SetSize(const FVector4& InNewSize);
	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
