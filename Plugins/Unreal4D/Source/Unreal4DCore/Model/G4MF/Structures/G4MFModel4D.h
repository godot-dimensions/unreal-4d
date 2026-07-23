#pragma once

#include "G4MFFileReference4D.h"

#include "G4MFModel4D.generated.h"

class UScene4DComponent;
class UG4MFDocument4D;

// UG4MFModel4D represents a reference to another self-contained model file that can be used in
// the scene graph, such as other G4MF files, OFF files, glTF™ files, OCIF files, or any other format.
// These models are then used by nodes via UG4MFModelInstance4D, where each node using
// the model becomes a new instance of the model's tree of nodes, and the host G4MF's
// node properties override the properties of the instantiated model's root node.
// This allows for modularity and reusability of models across different scenes and applications.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFModel4D : public UG4MFFileReference4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UG4MFDocument4D> ModelG4MFDocument;

	UPROPERTY()
	TObjectPtr<UG4MFState4D> ModelG4MFState;

	FString GetInferredMimeType() const;
	virtual bool ImportParseFileData(UG4MFState4D* InG4MFState) override;
	UScene4DComponent* ImportInstantiateModel(const UG4MFState4D* InG4MFState, AActor* InActor);
	static int32 ExportPackNodesIntoModel(UG4MFDocument4D* InG4MFDocument, UG4MFState4D* InG4MFState, UScene4DComponent* InRootComponent, const bool InDeduplicate = true);
	virtual bool ExportSerializeFileData(UG4MFState4D* InG4MFState, const bool bInDeduplicate = true, const int32 InBufferIndex = 0) override;

	static UG4MFModel4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
};
