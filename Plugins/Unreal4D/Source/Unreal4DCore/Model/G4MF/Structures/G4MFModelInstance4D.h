#pragma once

#include "G4MFItem4D.h"

#include "G4MFModelInstance4D.generated.h"

class UScene4DComponent;

// UG4MFModelInstance4D represents an instance of a UG4MFModel4D on a UG4MFNode4D.
// This is used to instantiate an entire model hierarchy, such as from an embedded G4MF file.
// The UG4MFModelInstance4D contains the index of the referenced UG4MFModel4D in the
// UG4MFState4D's G4MFFiles array, and may contain overrides for the nodes in the model,
// as well as additional children to add to the root of the instantiated model.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFModelInstance4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FString, FInt32Array4D> NodeAdditionalChildren;

	// TODO: Replace the raw JSON values with an Unreal-serializable override
	// representation before nested-model overrides are considered implemented.
	// As written, overrides do not survive saving the containing state to a .uasset.
	TMap<FString, TSharedPtr<FJsonObject>> NodeOverrides;

	UPROPERTY()
	int32 ModelFileIndex = -1;

	UScene4DComponent* ImportInstantiateModelComponent(const UG4MFState4D* InG4MFState, AActor* InActor) const;

	static UG4MFModelInstance4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
