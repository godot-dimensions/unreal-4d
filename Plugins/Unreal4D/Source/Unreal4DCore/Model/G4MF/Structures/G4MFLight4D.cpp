#include "G4MFLight4D.h"

#include "../G4MFState4D.h"

UG4MFLight4D* UG4MFLight4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFLight4D* light = NewObject<UG4MFLight4D>(InG4MFState, uniqueName);
	light->ReadItemEntriesFromJson(InJson);
	// UG4MFLight4D-specific code.
	if (InJson->HasField(TEXT("type")))
	{
		light->LightType = InJson->GetStringField(TEXT("type"));
	}
	if (InJson->HasField(TEXT("color")))
	{
		light->Color = UG4MFItem4D::JsonArrayToColor(InJson->GetArrayField(TEXT("color")));
	}
	if (InJson->HasField(TEXT("coneInnerAngle")))
	{
		light->ConeInnerAngle = InJson->GetNumberField(TEXT("coneInnerAngle"));
	}
	if (InJson->HasField(TEXT("coneOuterAngle")))
	{
		light->ConeOuterAngle = InJson->GetNumberField(TEXT("coneOuterAngle"));
	}
	if (InJson->HasField(TEXT("intensity")))
	{
		light->Intensity = InJson->GetNumberField(TEXT("intensity"));
	}
	if (InJson->HasField(TEXT("range")))
	{
		light->Range = InJson->GetNumberField(TEXT("range"));
	}
	return light;
}

TSharedPtr<FJsonObject> UG4MFLight4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (Color != FLinearColor(1.0f, 1.0f, 1.0f))
	{
		json->SetArrayField(TEXT("color"), UG4MFItem4D::ColorToJsonArray(Color, false));
	}
	if (ConeInnerAngle != 0.0)
	{
		json->SetNumberField(TEXT("coneInnerAngle"), ConeInnerAngle);
	}
	if (ConeOuterAngle != 0.7853981633974483)
	{
		json->SetNumberField(TEXT("coneOuterAngle"), ConeOuterAngle);
	}
	json->SetNumberField(TEXT("intensity"), Intensity);
	if (FMath::IsFinite(Range))
	{
		json->SetNumberField(TEXT("range"), Range);
	}
	json->SetStringField(TEXT("type"), LightType);
	return json;
}
