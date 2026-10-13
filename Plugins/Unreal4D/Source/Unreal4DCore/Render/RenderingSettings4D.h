#pragma once

#include "Engine/DeveloperSettings.h"

#include "RenderingSettings4D.generated.h"

class UMaterialInterface;

/** Project-wide defaults for 4D rendering. */
UCLASS(BlueprintType, Config = Game, DefaultConfig, meta = (DisplayName = "Unreal 4D Rendering"))
class UNREAL4DCORE_API URenderingSettings4D : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URenderingSettings4D();
	virtual void PostInitProperties() override;

	UFUNCTION(BlueprintPure, Category = "Unreal 4D|Rendering")
	static URenderingSettings4D* Get();

	UFUNCTION(BlueprintPure, Category = "Unreal 4D|Rendering")
	UMaterialInterface* GetDefaultProjectionMaterial();

	/** Used when a mesh has no ProjectionMaterial override. Keep custom primitive data indices 0-3 for color and opacity. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Projection")
	TSoftObjectPtr<UMaterialInterface> DefaultProjectionMaterial;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> LoadedProjectionMaterial;
};
