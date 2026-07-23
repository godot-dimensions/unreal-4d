#include "G4MFNodePhysicsMotion4D.h"

#include "../../G4MFState4D.h"

UG4MFNodePhysicsMotion4D* UG4MFNodePhysicsMotion4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFNodePhysicsMotion4D* ret = NewObject<UG4MFNodePhysicsMotion4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFNodePhysicsMotion4D-specific code.
	if (InJson && InJson->HasField(TEXT("gravityFactor")))
	{
		ret->GravityFactor = InJson->GetNumberField(TEXT("gravityFactor"));
	}
	if (InJson && InJson->HasField(TEXT("linearVelocity")))
	{
		const TArray<TSharedPtr<FJsonValue>> linearVelocityArray = InJson->GetArrayField(TEXT("linearVelocity"));
		ret->LinearVelocity = UG4MFItem4D::JsonArrayToVector4(linearVelocityArray);
	}
	if (InJson && InJson->HasField(TEXT("mass")))
	{
		ret->Mass = InJson->GetNumberField(TEXT("mass"));
	}
	if (InJson && InJson->HasField(TEXT("type")))
	{
		ret->MotionType = InJson->GetStringField(TEXT("type"));
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFNodePhysicsMotion4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (GravityFactor != 1.0)
	{
		json->SetNumberField(TEXT("gravityFactor"), GravityFactor);
	}
	if (LinearVelocity != FVector4(0.0, 0.0, 0.0, 0.0))
	{
		TArray<TSharedPtr<FJsonValue>> linearVelocityArray = UG4MFItem4D::Vector4ToJsonArray(LinearVelocity);
		json->SetArrayField(TEXT("linearVelocity"), linearVelocityArray);
	}
	if (Mass != 1.0)
	{
		json->SetNumberField(TEXT("mass"), Mass);
	}
	json->SetStringField(TEXT("type"), MotionType);
	return json;
}
