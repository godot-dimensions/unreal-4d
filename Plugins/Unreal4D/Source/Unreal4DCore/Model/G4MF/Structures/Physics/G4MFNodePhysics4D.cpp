#include "G4MFNodePhysics4D.h"

#include "../../G4MFState4D.h"

bool UG4MFNodePhysics4D::IsEmptyOfMotionAndShapes() const
{
	return !IsValid(Motion) && ColliderShapeIndex == -1 && TriggerShapeIndex == -1;
}

UG4MFNodePhysics4D* UG4MFNodePhysics4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InG4MFState == nullptr || InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFNodePhysics4D* ret = NewObject<UG4MFNodePhysics4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFNodePhysics4D-specific code.
	if (InJson->HasField(TEXT("motion")))
	{
		const TSharedPtr<FJsonObject> motionJson = InJson->GetObjectField(TEXT("motion"));
		ret->Motion = UG4MFNodePhysicsMotion4D::FromJson(InG4MFState, motionJson.Get());
	}
	if (InJson->HasField(TEXT("collider")))
	{
		const TSharedPtr<FJsonObject> colliderJson = InJson->GetObjectField(TEXT("collider"));
		if (colliderJson == nullptr || !colliderJson.IsValid())
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Collider field is not an object."));
			return ret;
		}
		if (colliderJson && colliderJson->HasField(TEXT("shape")))
		{
			ret->ColliderShapeIndex = colliderJson->GetIntegerField(TEXT("shape"));
		}
	}
	if (InJson->HasField(TEXT("trigger")))
	{
		const TSharedPtr<FJsonObject> triggerJson = InJson->GetObjectField(TEXT("trigger"));
		if (triggerJson == nullptr || !triggerJson.IsValid())
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Trigger field is not an object."));
			return ret;
		}
		if (triggerJson->HasField(TEXT("shape")))
		{
			ret->TriggerShapeIndex = triggerJson->GetIntegerField(TEXT("shape"));
		}
		if (triggerJson->HasField(TEXT("nodes")))
		{
			const TArray<TSharedPtr<FJsonValue>> nodesJsonArray = triggerJson->GetArrayField(TEXT("nodes"));
			ret->TriggerNodeIndices = UG4MFItem4D::JsonArrayToInt32Array(nodesJsonArray);
		}
	}

	return ret;
}

TSharedPtr<FJsonObject> UG4MFNodePhysics4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (Motion != nullptr && IsValid(Motion))
	{
		json->SetObjectField(TEXT("motion"), Motion->ToJson());
	}
	if (ColliderShapeIndex > -1)
	{
		TSharedPtr<FJsonObject> colliderJson = MakeShared<FJsonObject>();
		colliderJson->SetNumberField(TEXT("shape"), ColliderShapeIndex);
		json->SetObjectField(TEXT("collider"), colliderJson);
	}
	const bool bUseCompoundTriggerNodes = TriggerNodeIndices.Num() > 0 || IsEmptyOfMotionAndShapes();
	if (TriggerShapeIndex > -1 || bUseCompoundTriggerNodes)
	{
		TSharedPtr<FJsonObject> triggerJson = MakeShared<FJsonObject>();
		if (TriggerShapeIndex > -1)
		{
			triggerJson->SetNumberField(TEXT("shape"), TriggerShapeIndex);
		}
		if (bUseCompoundTriggerNodes)
		{
			triggerJson->SetArrayField(TEXT("nodes"), UG4MFItem4D::Int32ArrayToJsonArray(TriggerNodeIndices));
		}
		json->SetObjectField(TEXT("trigger"), triggerJson);
	}
	return json;
}
