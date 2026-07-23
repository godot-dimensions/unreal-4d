#pragma once

#include "G4MFMaterialChannel4D.h"

#include "G4MFMaterial4D.generated.h"

class UMaterial4D;
class UTetraMaterial4D;

// UG4MFMaterial4D is a class for materials in the G4MF format. It is used to
// define the appearance of a UG4MFMeshSurface4D inside of a UG4MFMesh4D.
// A G4MF material has multiple channels represented by UG4MFMaterialChannel4D objects.
// Each channel may be used to define a different property of the material, such as
// the BaseColorChannel, NormalChannel, EmissiveChannel, and OrmChannel.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMaterial4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UG4MFMaterialChannel4D> BaseColorChannel;

	UPROPERTY()
	TObjectPtr<UG4MFMaterialChannel4D> EmissiveChannel;

	UPROPERTY()
	TObjectPtr<UG4MFMaterialChannel4D> NormalChannel;

	UPROPERTY()
	TObjectPtr<UG4MFMaterialChannel4D> OrmChannel;

	void FlipSimplexWindingOrder(UG4MFState4D* InG4MFState);
	bool IsEqualExact(const UG4MFMaterial4D* InOther) const;

	UTetraMaterial4D* ImportGenerateTetraMaterial(const UG4MFState4D* InG4MFState, UObject* InOuter) const;
	static int32 ExportConvertMaterialIntoState(UG4MFState4D* InG4MFState, const UMaterial4D* InMaterial, const bool InDeduplicate = true);

	static UG4MFMaterial4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
