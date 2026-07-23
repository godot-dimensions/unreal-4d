#pragma once

#include "G4MFItem4D.h"

#include "G4MFMeshInstance4D.generated.h"

class UMesh4DComponent;

// UG4MFMeshInstance4D represents an instance of a UG4MFMesh4D on a UG4MFNode4D.
// This is used to instantiate a mesh on a node, which allows the mesh to be
// rendered in Unreal at the node's 4D transform.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFMeshInstance4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<int32> MaterialIndices;

	UPROPERTY()
	int32 MeshIndex = -1;

	UMesh4DComponent* ImportGenerateMeshComponent(UG4MFState4D* InG4MFState, AActor* InActor) const;
	static UG4MFMeshInstance4D* ExportConvertMeshComponent(UG4MFState4D* InG4MFState, const UMesh4DComponent* InMeshComponent);

	static UG4MFMeshInstance4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
