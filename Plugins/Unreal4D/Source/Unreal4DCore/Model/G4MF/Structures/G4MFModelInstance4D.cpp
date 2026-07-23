#include "G4MFModelInstance4D.h"

#include "../../../Scene/Components/Scene4DComponent.h"
#include "../G4MFState4D.h"

UScene4DComponent* UG4MFModelInstance4D::ImportInstantiateModelComponent(const UG4MFState4D* InG4MFState, AActor* InActor) const
{
	if (InG4MFState == nullptr || InActor == nullptr) { return nullptr; }
	const FName desiredName = FName(GetItemName());
	if (InG4MFState->G4MFFiles.IsValidIndex(ModelFileIndex))
	{
		UG4MFModel4D* g4mfModel = Cast<UG4MFModel4D>(InG4MFState->G4MFFiles[ModelFileIndex]);
		if (IsValid(g4mfModel))
		{
			return g4mfModel->ImportInstantiateModel(InG4MFState, InActor);
		}
	}
	return nullptr;
}

UG4MFModelInstance4D* UG4MFModelInstance4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFModelInstance4D* ret = NewObject<UG4MFModelInstance4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFModelInstance4D-specific code.
	if (InJson->HasField(TEXT("file")))
	{
		ret->ModelFileIndex = InJson->GetIntegerField(TEXT("file"));
	}
	if (InJson->HasField(TEXT("nodeAdditionalChildren")))
	{
		TSharedPtr<FJsonObject> nodeAdditionalChildrenJson = InJson->GetObjectField(TEXT("nodeAdditionalChildren"));
		for (const auto& pair : nodeAdditionalChildrenJson->Values)
		{
			const FString& nodeName = pair.Key;
			const TArray<TSharedPtr<FJsonValue>>& childrenJsonArray = pair.Value->AsArray();
			FInt32Array4D childrenIndices;
			for (const TSharedPtr<FJsonValue>& childJsonValue : childrenJsonArray)
			{
				childrenIndices.Data.Add(childJsonValue->AsNumber());
			}
			ret->NodeAdditionalChildren.Add(nodeName, childrenIndices);
		}
	}
	if (InJson->HasField(TEXT("nodeOverrides")))
	{
		TSharedPtr<FJsonObject> nodeOverridesJson = InJson->GetObjectField(TEXT("nodeOverrides"));
		for (const auto& pair : nodeOverridesJson->Values)
		{
			const FString& nodeName = pair.Key;
			const TSharedPtr<FJsonObject>& overrideJsonObject = pair.Value->AsObject();
			ret->NodeOverrides.Add(nodeName, overrideJsonObject);
		}
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFModelInstance4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (ModelFileIndex >= 0)
	{
		json->SetNumberField(TEXT("file"), ModelFileIndex);
	}
	if (NodeAdditionalChildren.Num() > 0)
	{
		TSharedPtr<FJsonObject> nodeAdditionalChildrenJson = MakeShared<FJsonObject>();
		for (const auto& pair : NodeAdditionalChildren)
		{
			const FString& nodeName = pair.Key;
			const FInt32Array4D& childrenIndices = pair.Value;
			TArray<TSharedPtr<FJsonValue>> childrenJsonArray;
			for (const int32 childIndex : childrenIndices.Data)
			{
				childrenJsonArray.Add(MakeShared<FJsonValueNumber>(childIndex));
			}
			nodeAdditionalChildrenJson->SetArrayField(nodeName, childrenJsonArray);
		}
		json->SetObjectField(TEXT("nodeAdditionalChildren"), nodeAdditionalChildrenJson);
	}
	if (NodeOverrides.Num() > 0)
	{
		TSharedPtr<FJsonObject> nodeOverridesJson = MakeShared<FJsonObject>();
		for (const auto& pair : NodeOverrides)
		{
			const FString& nodeName = pair.Key;
			const TSharedPtr<FJsonObject>& overrideJsonObject = pair.Value;
			nodeOverridesJson->SetObjectField(nodeName, overrideJsonObject);
		}
		json->SetObjectField(TEXT("nodeOverrides"), nodeOverridesJson);
	}
	return json;
}
