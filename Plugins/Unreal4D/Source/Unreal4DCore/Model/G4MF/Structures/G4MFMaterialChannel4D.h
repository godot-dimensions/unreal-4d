#pragma once

#include "G4MFMeshSurfaceBinding4D.h"

#include "G4MFMaterialChannel4D.generated.h"

class UG4MFMaterial4D;

// UG4MFMaterialChannel4D is a class for material channels inside of a UG4MFMaterial4D.
// A material channel is a single property of a material, such as BaseColorChannel,
// NormalChannel, EmissiveChannel, and OrmChannel. The colors in a material channel
// may represent actual colors, or may be more abstract data, such as a normal map's
// color values encoding the normal vectors of a surface.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMaterialChannel4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	// The element map is used for things like vertex colors, cell colors, etc.
	UPROPERTY()
	TObjectPtr<UG4MFMeshSurfaceBinding4D> ElementMapBinding;

	UPROPERTY()
	TObjectPtr<UG4MFMeshSurfaceBinding4D> TextureMapBinding;

	UPROPERTY()
	FLinearColor Factor = FLinearColor(-1, -1, -1, -1);

	UPROPERTY()
	int32 TextureIndex = -1;

	void FlipSimplexWindingOrder(UG4MFState4D* InG4MFState);
	bool IsEqualExact(const UG4MFMaterialChannel4D* InOther) const;

	static UG4MFMaterialChannel4D* FromJson(UG4MFState4D* InG4MFState, UG4MFMaterial4D* InOwningMaterial, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
