#pragma once

#include "../Base/Material4D.h"

#include "TetraMaterial4D.generated.h"

UENUM(BlueprintType, Category = "Unreal 4D|Material|Tetra")
enum class ETetraColorSource4D : uint8
{
	SingleColor = 0,
	PerCellOnly = 2,
	Texture3DCellUVWOnly = 3,
	Texture4DDirectOnly = 4,
	PerCellAndSingle = 6,
	Texture3DCellUVWAndSingle = 7,
	Texture4DDirectAndSingle = 8,
};

UCLASS(BlueprintType, Category = "Unreal 4D|Material|Tetra")
class UNREAL4DCORE_API UTetraMaterial4D : public UMaterial4D
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, Category="Unreal 4D|Material|Tetra")
	ETetraColorSource4D AlbedoSource = ETetraColorSource4D::SingleColor;

	static EColorSourceFlags4D TetraSourceToFlags(ETetraColorSource4D InTetraSource);

public:
	// Note: Texture3D support requires proper Unreal material/rendering system integration
	// UTexture* AlbedoTexture3D;

	virtual void MergeWith(UMaterial4D* InMaterial, int32 FirstItemCount, int32 SecondItemCount) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Unreal 4D|Material|Tetra")
	ETetraColorSource4D GetAlbedoSource() const;

	UFUNCTION(BlueprintCallable, Category="Unreal 4D|Material|Tetra")
	void SetAlbedoSource(ETetraColorSource4D InAlbedoSource);

	// Texture3D support placeholder - requires proper Unreal material system integration
	// UTexture* GetAlbedoTexture3D() const;
	// void SetAlbedoTexture3D(UTexture* InTexture3D);

	UTetraMaterial4D();
};
