#pragma once

#include "G4MFFileReference4D.h"

#include "G4MFTexture4D.generated.h"

// UG4MFTexture4D is a class for textures in the G4MF format. It is used to define
// the texture of a UG4MFMaterialChannel4D, which affects the appearance of the
// UG4MFMaterial4D and therefore any UG4MFMeshSurface4D that uses the material.
//
// Each texture may contain multiple images, which may construct textures of different
// dimensions based on the size property. Images may be cut and assembled together
// into higher-dimensional textures than the image formats themselves support, such as
// using multiple 2D images to construct a 3D texture, until all pixels in the texture
// are filled. Additionally, images of unknown formats should be skipped, allowing further
// images may be used as fallbacks, or the placeholder color if no images are usable.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFTexture4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	// Texture pixel data properties.
	UPROPERTY()
	TArray<int32> ImageFileIndices;
	UPROPERTY()
	TArray<int32> TextureSize;
	UPROPERTY()
	FLinearColor PlaceholderColor;

	// Texture sampler properties.
	UPROPERTY()
	FString SamplerMagFilter = TEXT("linear");
	UPROPERTY()
	FString SamplerMinFilter = TEXT("linear");
	UPROPERTY()
	FString SamplerMipmapFilter = TEXT("linear");
	UPROPERTY()
	TArray<FString> SamplerWrapModes;

	static UG4MFTexture4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
