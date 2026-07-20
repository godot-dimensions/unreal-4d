#pragma once

#include "../../../Unreal4DDefines.h"

#include "UObject/NoExportTypes.h"

#include "Material4D.generated.h"

UENUM(BlueprintType, Category = "Unreal 4D|Material", meta = (Bitflags))
enum class EColorSourceFlags4D : uint8
{
	None = 0,
	SingleColor = 1 << 0,
	PerCell = 1 << 1,
	Texture3DCellUVW = 1 << 2,
	Texture4DDirect = 1 << 3,
	UsesColorArray = PerCell,
	UsesTexture3D = Texture3DCellUVW,
	UsesTexture4D = Texture4DDirect
};

ENUM_CLASS_FLAGS(EColorSourceFlags4D);

UCLASS(BlueprintType, Category = "Unreal 4D|Material")
class UNREAL4DCORE_API UMaterial4D : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Material")
	EColorSourceFlags4D AlbedoSourceFlags = EColorSourceFlags4D::SingleColor;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Material")
	TArray<FLinearColor> AlbedoColorArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Material")
	FLinearColor AlbedoColor = FLinearColor(1, 1, 1, 1);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Unreal 4D|Material")
	bool IsDefaultMaterial() const;

	UFUNCTION(BlueprintCallable, Category = "Unreal 4D|Material")
	virtual void MergeWith(UMaterial4D* InMaterial, int32 FirstItemCount, int32 SecondItemCount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Unreal 4D|Material")
	EColorSourceFlags4D GetAlbedoSourceFlags() const;

	UFUNCTION(BlueprintCallable, Category = "Unreal 4D|Material")
	void SetAlbedoSourceFlags(EColorSourceFlags4D InAlbedoSourceFlags);

	UFUNCTION(BlueprintCallable, Category = "Unreal 4D|Material")
	void AppendAlbedoColor(const FLinearColor& InAlbedoColor);

	UFUNCTION(BlueprintCallable, Category = "Unreal 4D|Material")
	void ResizeAlbedoColorArray(int32 InSize, const FLinearColor& InFillColor = FLinearColor(1, 1, 1, 1));

	UMaterial4D() = default;
};
