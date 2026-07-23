#include "G4MFMeshInstance4D.h"

#include "../../../Scene/Components/Mesh4DComponent.h"
#include "../../Mesh/Base/Mesh4D.h"
#include "../G4MFState4D.h"

UMesh4DComponent* UG4MFMeshInstance4D::ImportGenerateMeshComponent(UG4MFState4D* InG4MFState, AActor* InActor) const
{
	ensure(InG4MFState != nullptr && InActor != nullptr);
	const FName desiredName = FName(GetItemName());
	UMesh4DComponent* ret = NewObject<UMesh4DComponent>(InActor, desiredName);
	if (InG4MFState->G4MFMeshes.IsValidIndex(MeshIndex))
	{
		if (InG4MFState->UnrealMeshAssets.IsValidIndex(MeshIndex) && IsValid(InG4MFState->UnrealMeshAssets[MeshIndex]))
		{
			ret->Mesh4D = InG4MFState->UnrealMeshAssets[MeshIndex];
		}
		else
		{
			const UG4MFMesh4D* g4mfMesh = InG4MFState->G4MFMeshes[MeshIndex];
			if (IsValid(g4mfMesh))
			{
				ret->Mesh4D = g4mfMesh->ImportGenerateMesh(InG4MFState, ret);
				InG4MFState->UnrealMeshAssets.SetNum(InG4MFState->G4MFMeshes.Num());
				InG4MFState->UnrealMeshAssets[MeshIndex] = ret->Mesh4D;
			}
		}
	}
	for (int32 materialIndex : MaterialIndices)
	{
		if (InG4MFState->G4MFMaterials.IsValidIndex(materialIndex))
		{
			const UG4MFMaterial4D* material = InG4MFState->G4MFMaterials[materialIndex];
			if (IsValid(material))
			{
				// TODO: Implement per-instance 4D material overrides.
				//ret->Material4D = material->GenerateMaterial(InG4MFState, ret);
			}
		}
	}
	return ret;
}

UG4MFMeshInstance4D* UG4MFMeshInstance4D::ExportConvertMeshComponent(UG4MFState4D* InG4MFState, const UMesh4DComponent* InMeshComponent)
{
	if (InG4MFState == nullptr || InMeshComponent == nullptr) { return nullptr; }
	const FName desiredName = FName(InMeshComponent->GetName());
	UG4MFMeshInstance4D* ret = NewObject<UG4MFMeshInstance4D>(InG4MFState, desiredName);
	UMesh4D* mesh = InMeshComponent->Mesh4D;
	// Convert the mesh reference.
	if (IsValid(mesh))
	{
		ret->MeshIndex = UG4MFMesh4D::ExportConvertMeshIntoState(InG4MFState, mesh, true);
		// TODO: Support material overrides on mesh instances.
		UMaterial4D* material = nullptr; // InMeshComponent->GetMaterial();
		if (IsValid(material))
		{
			ret->MaterialIndices.Add(UG4MFMaterial4D::ExportConvertMaterialIntoState(InG4MFState, material, true));
		}
	}
	return ret;
}

UG4MFMeshInstance4D* UG4MFMeshInstance4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InG4MFState == nullptr || InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMeshInstance4D* ret = NewObject<UG4MFMeshInstance4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFMeshInstance4D-specific code.
	if (InJson->HasField(TEXT("materials")))
	{
		TArray<TSharedPtr<FJsonValue>> materialsJsonArray = InJson->GetArrayField(TEXT("materials"));
		for (const TSharedPtr<FJsonValue>& materialJsonValue : materialsJsonArray)
		{
			ret->MaterialIndices.Add(materialJsonValue->AsNumber());
		}
	}
	if (InJson->HasField(TEXT("mesh")))
	{
		ret->MeshIndex = InJson->GetIntegerField(TEXT("mesh"));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFMeshInstance4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (MaterialIndices.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> materialsJsonArray;
		materialsJsonArray.SetNum(MaterialIndices.Num());
		for (int32 i = 0; i < MaterialIndices.Num(); ++i)
		{
			materialsJsonArray[i] = MakeShared<FJsonValueNumber>(MaterialIndices[i]);
		}
		json->SetArrayField(TEXT("materials"), materialsJsonArray);
	}
	if (MeshIndex >= 0)
	{
		json->SetNumberField(TEXT("mesh"), MeshIndex);
	}
	return json;
}
