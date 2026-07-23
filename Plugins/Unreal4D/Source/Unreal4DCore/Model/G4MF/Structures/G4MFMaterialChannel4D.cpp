#include "G4MFMaterialChannel4D.h"

#include "../G4MFState4D.h"
#include "G4MFMaterial4D.h"

void UG4MFMaterialChannel4D::FlipSimplexWindingOrder(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return; }
	if (IsValid(ElementMapBinding))
	{
		ElementMapBinding->FlipSimplexWindingOrder(InG4MFState);
	}
	if (IsValid(TextureMapBinding))
	{
		TextureMapBinding->FlipSimplexWindingOrder(InG4MFState);
	}
}

bool UG4MFMaterialChannel4D::IsEqualExact(const UG4MFMaterialChannel4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	if (Factor != InOther->Factor)
	{
		return false;
	}
	if (TextureIndex != InOther->TextureIndex)
	{
		return false;
	}
	const bool selfElemMapValid = IsValid(ElementMapBinding);
	const bool otherElemMapValid = IsValid(InOther->ElementMapBinding);
	if (selfElemMapValid != otherElemMapValid)
	{
		return false;
	}
	if (selfElemMapValid)
	{
		if (!ElementMapBinding->IsEqualExact(InOther->ElementMapBinding))
		{
			return false;
		}
	}
	const bool selfTexMapValid = IsValid(TextureMapBinding);
	const bool otherTexMapValid = IsValid(InOther->TextureMapBinding);
	if (selfTexMapValid != otherTexMapValid)
	{
		return false;
	}
	if (selfTexMapValid)
	{
		if (!TextureMapBinding->IsEqualExact(InOther->TextureMapBinding))
		{
			return false;
		}
	}
	return true;
}

UG4MFMaterialChannel4D* UG4MFMaterialChannel4D::FromJson(UG4MFState4D* InG4MFState, UG4MFMaterial4D* InOwningMaterial, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	// UG4MFMaterialChannel4D is owned by sub-objects of the state, but names MUST still be file-unique as per the spec.
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMaterialChannel4D* ret = NewObject<UG4MFMaterialChannel4D>(InOwningMaterial, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFMaterialChannel4D-specific code.
	if (InJson->HasField(TEXT("elementMap")))
	{
		FJsonObject* elementMapJsonObjectPtr = InJson->GetObjectField(TEXT("elementMap")).Get();
		ret->ElementMapBinding = UG4MFMeshSurfaceBinding4D::FromJson(InG4MFState, ret, elementMapJsonObjectPtr);
	}
	if (InJson->HasField(TEXT("factor")))
	{
		TArray<TSharedPtr<FJsonValue>> factorArray = InJson->GetArrayField(TEXT("factor"));
		FLinearColor factorColor = FLinearColor::White;
		if (factorArray.Num() == 0)
		{
			factorColor = FLinearColor(-1.0f, -1.0f, -1.0f, -1.0f);
		}
		else
		{
			factorColor = UG4MFItem4D::JsonArrayToColor(factorArray);
		}
		ret->Factor = factorColor;
	}
	if (InJson->HasField(TEXT("texture")))
	{
		ret->TextureIndex = InJson->GetIntegerField(TEXT("texture"));
	}
	if (InJson->HasField(TEXT("textureMap")))
	{
		FJsonObject* textureMapJsonObjectPtr = InJson->GetObjectField(TEXT("textureMap")).Get();
		ret->TextureMapBinding = UG4MFMeshSurfaceBinding4D::FromJson(InG4MFState, ret, textureMapJsonObjectPtr);
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFMaterialChannel4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = UG4MFItem4D::WriteItemEntriesToJson();
	if (IsValid(ElementMapBinding))
	{
		json->SetObjectField(TEXT("elementMap"), ElementMapBinding->ToJson());
	}
	// Store factor if it's not the default white or if alpha differs from 1.0
	if (Factor.R > -1.0f && !Factor.Equals(FLinearColor::White))
	{
		TArray<TSharedPtr<FJsonValue>> factorArray;
		if (FMath::IsNearlyEqual(Factor.A, 1.0f))
		{
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.R));
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.G));
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.B));
		}
		else
		{
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.R));
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.G));
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.B));
			factorArray.Add(MakeShared<FJsonValueNumber>(Factor.A));
		}
		json->SetArrayField(TEXT("factor"), factorArray);
	}
	if (TextureIndex >= 0)
	{
		json->SetNumberField(TEXT("texture"), TextureIndex);
	}
	if (IsValid(TextureMapBinding))
	{
		json->SetObjectField(TEXT("textureMap"), TextureMapBinding->ToJson());
	}
	return json;
}
