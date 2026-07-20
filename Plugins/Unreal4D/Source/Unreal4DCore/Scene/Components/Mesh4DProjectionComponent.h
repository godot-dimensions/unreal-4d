/*
 * GPU-backed 4D mesh projection component.
 *
 * This component is created and controlled by UMesh4DComponent when mesh
 * projection rendering is enabled. It owns a separate primitive scene proxy and
 * bounds so projection visibility is independent from slice visibility.
 */

#pragma once

#include "../../Unreal4DDefines.h"

#include "Components/PrimitiveComponent.h"

#include "Mesh4DProjectionComponent.generated.h"

class UMesh4DComponent;
class UMaterialInterface;

UCLASS(BlueprintType, Category = "Unreal 4D|Component|Mesh", NotBlueprintable, meta=(BlueprintSpawnableComponent = false))
class UNREAL4DCORE_API UMesh4DProjectionComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UMesh4DProjectionComponent(const FObjectInitializer& InInit);

	void SetSourceComponent(UMesh4DComponent* InSourceComponent);
	void SetProjectionEnabled(bool bInProjectionEnabled);
	void SetProjectionAppearance(const FLinearColor& InColor, float InOpacity, float InScale, float InDistance);
	void SetProjectionBoundsTransform(const FTransform& InBoundsTransform);

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	virtual int32 GetNumMaterials() const override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMesh4DComponent> SourceComponent = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> Material = nullptr;

	void UpdateCustomPrimitiveData();

	FTransform CachedBoundsTransform = FTransform::Identity;
	FLinearColor ProjectionColor = FLinearColor::White;
	float ProjectionOpacity = 1.0f;
	float ProjectionScale = 1.0f;
	float ProjectionDistance = 2000.0f;
	bool bProjectionEnabled = false;
};
