#include "G4MFMaterial4D.h"

#include "../../Mesh/Tetra/TetraMaterial4D.h"
#include "../G4MFState4D.h"
#include "G4MFMaterialChannel4D.h"

void UG4MFMaterial4D::FlipSimplexWindingOrder(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return; }
	if (IsValid(BaseColorChannel))
	{
		BaseColorChannel->FlipSimplexWindingOrder(InG4MFState);
	}
	if (IsValid(EmissiveChannel))
	{
		EmissiveChannel->FlipSimplexWindingOrder(InG4MFState);
	}
	if (IsValid(NormalChannel))
	{
		NormalChannel->FlipSimplexWindingOrder(InG4MFState);
	}
	if (IsValid(OrmChannel))
	{
		OrmChannel->FlipSimplexWindingOrder(InG4MFState);
	}
}

bool UG4MFMaterial4D::IsEqualExact(const UG4MFMaterial4D* InOther) const
{
	if (InOther == nullptr || !IsValid(InOther)) return false;
	if ((IsValid(BaseColorChannel) != IsValid(InOther->BaseColorChannel)) ||
			(IsValid(EmissiveChannel) != IsValid(InOther->EmissiveChannel)) ||
			(IsValid(NormalChannel) != IsValid(InOther->NormalChannel)) ||
			(IsValid(OrmChannel) != IsValid(InOther->OrmChannel)))
			{
		return false;
	}
	if (IsValid(BaseColorChannel) && !BaseColorChannel->IsEqualExact(InOther->BaseColorChannel))
	{
		return false;
	}
	if (IsValid(EmissiveChannel) && !EmissiveChannel->IsEqualExact(InOther->EmissiveChannel))
	{
		return false;
	}
	if (IsValid(NormalChannel) && !NormalChannel->IsEqualExact(InOther->NormalChannel))
	{
		return false;
	}
	if (IsValid(OrmChannel) && !OrmChannel->IsEqualExact(InOther->OrmChannel))
	{
		return false;
	}
	return true;
}

UTetraMaterial4D* UG4MFMaterial4D::ImportGenerateTetraMaterial(const UG4MFState4D* InG4MFState, UObject* InOuter) const
{
	if (InG4MFState == nullptr) { return nullptr; }
	const FName desiredName = FName(GetItemName());
	UTetraMaterial4D* ret = NewObject<UTetraMaterial4D>(InOuter, desiredName);
	if (IsValid(BaseColorChannel))
	{
		const bool bHasSingleBaseColor = BaseColorChannel->Factor.R > -1 && !BaseColorChannel->Factor.Equals(FLinearColor(1, 1, 1, 1));
		if (bHasSingleBaseColor)
		{
			ret->AlbedoColor = BaseColorChannel->Factor;
		}
		if (IsValid(BaseColorChannel->ElementMapBinding))
		{
			const TArray<FLinearColor> colorValues = BaseColorChannel->ElementMapBinding->LoadValuesAsColors(InG4MFState);
			if (BaseColorChannel->ElementMapBinding->PerSimplexAccessorIndex >= 0)
			{
				TArray<int32> simplexIndices = BaseColorChannel->ElementMapBinding->LoadPerSimplexIndices(InG4MFState);
				TArray<FLinearColor> simplexColors;
				simplexColors.SetNum(simplexIndices.Num());
				for (int32 i = 0; i < simplexIndices.Num(); i++)
				{
					const int32 colorIndex = simplexIndices[i];
					if (colorValues.IsValidIndex(colorIndex))
					{
						simplexColors[i] = colorValues[colorIndex];
					}
				}

				ret->AlbedoColorArray = simplexColors;
				if (bHasSingleBaseColor)
				{
					ret->SetAlbedoSource(ETetraColorSource4D::PerCellAndSingle);
				}
				else
				{
					ret->SetAlbedoSource(ETetraColorSource4D::PerCellOnly);
				}
			}
		}
	}
	if (IsValid(EmissiveChannel))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMaterial4D::GenerateTetraMaterial: Emissive maps are not supported yet."));
	}
	if (IsValid(NormalChannel))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMaterial4D::GenerateTetraMaterial: Normal maps are not supported yet."));
	}
	if (IsValid(OrmChannel))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UG4MFMaterial4D::GenerateTetraMaterial: ORM maps are not supported yet."));
	}
	return ret;
}

int32 UG4MFMaterial4D::ExportConvertMaterialIntoState(UG4MFState4D* InG4MFState, const UMaterial4D* InMaterial, const bool bInDeduplicate)
{
	if (InG4MFState == nullptr || InMaterial == nullptr) { return -1; }
	const EColorSourceFlags4D albedoFlags = InMaterial->GetAlbedoSourceFlags();
	const FName desiredName = FName(InMaterial->GetName());
	UG4MFMaterial4D* g4mfMaterial = NewObject<UG4MFMaterial4D>(InG4MFState, desiredName);
	{
		UG4MFMaterialChannel4D* baseColorChannel = nullptr;
		if (EnumHasAnyFlags(albedoFlags, EColorSourceFlags4D::UsesColorArray) && InMaterial->AlbedoColorArray.Num() > 0)
		{
			// Always perform this level of deduplication even if bInDeduplicate is false.
			TArray<FLinearColor> deduplicatedColors;
			TArray<uint32> deduplicatedIndices;
			deduplicatedIndices.SetNum(InMaterial->AlbedoColorArray.Num());
			TMap<FLinearColor, uint32> colorToIndexMap;
			for (int32 colorIndex = 0; colorIndex < InMaterial->AlbedoColorArray.Num(); colorIndex++)
			{
				const FLinearColor& color = InMaterial->AlbedoColorArray[colorIndex];
				if (colorToIndexMap.Contains(color))
				{
					deduplicatedIndices[colorIndex] = colorToIndexMap[color];
				}
				else
				{
					const int32 newIndex = deduplicatedColors.Num();
					deduplicatedColors.Add(color);
					colorToIndexMap.Add(color, newIndex);
					deduplicatedIndices[colorIndex] = newIndex;
				}
			}
			UG4MFMeshSurfaceBinding4D* baseColorElementBinding = NewObject<UG4MFMeshSurfaceBinding4D>(g4mfMaterial);
			baseColorElementBinding->ValuesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromColors(InG4MFState, deduplicatedColors, bInDeduplicate);
			const int32 indicesAccessorIndex = UG4MFAccessor4D::EncodeNewAccessorFromUints(InG4MFState, deduplicatedIndices, bInDeduplicate);
			if (EnumHasAnyFlags(albedoFlags, EColorSourceFlags4D::PerCell))
			{
				baseColorElementBinding->PerSimplexAccessorIndex = indicesAccessorIndex;
			}
			else
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Unsupported albedo source flag combination in material."));
			}
			if (baseColorChannel == nullptr)
			{
				baseColorChannel = NewObject<UG4MFMaterialChannel4D>(g4mfMaterial);
			}
			baseColorChannel->ElementMapBinding = baseColorElementBinding;
		}
		if (EnumHasAnyFlags(albedoFlags, EColorSourceFlags4D::SingleColor) && !FLinearColor(1, 1, 1, 1).Equals(InMaterial->AlbedoColor))
		{
			if (baseColorChannel == nullptr)
			{
				baseColorChannel = NewObject<UG4MFMaterialChannel4D>(g4mfMaterial);
			}
			baseColorChannel->Factor = InMaterial->AlbedoColor;
		}
		if (baseColorChannel != nullptr)
		{
			g4mfMaterial->BaseColorChannel = baseColorChannel;
		}
	}
	// Add the G4MFMaterial4D to the G4MFState4D, but check for duplicates first.
	const int32 stateMaterialCount = InG4MFState->G4MFMaterials.Num();
	if (bInDeduplicate)
	{
		for (int32 i = 0; i < stateMaterialCount; i++)
		{
			const UG4MFMaterial4D* StateMaterial = InG4MFState->G4MFMaterials[i];
			if (g4mfMaterial->IsEqualExact(StateMaterial))
			{
				// An identical material already exists in state, we can just use that.
				return i;
			}
		}
	}
	InG4MFState->G4MFMaterials.Add(g4mfMaterial);
	return stateMaterialCount;
}

UG4MFMaterial4D* UG4MFMaterial4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFMaterial4D* ret = NewObject<UG4MFMaterial4D>(InG4MFState, uniqueName);
	ret->ReadItemEntriesFromJson(InJson);
	// UG4MFMaterial4D-specific code.
	if (InJson->HasField(TEXT("baseColor")))
	{
		const FJsonObject* baseColorJsonPtr = InJson->GetObjectField(TEXT("baseColor")).Get();
		ret->BaseColorChannel = UG4MFMaterialChannel4D::FromJson(InG4MFState, ret, baseColorJsonPtr);
	}
	if (InJson->HasField(TEXT("emissive")))
	{
		const FJsonObject* emissiveJsonPtr = InJson->GetObjectField(TEXT("emissive")).Get();
		ret->EmissiveChannel = UG4MFMaterialChannel4D::FromJson(InG4MFState, ret, emissiveJsonPtr);
	}
	if (InJson->HasField(TEXT("normal")))
	{
		const FJsonObject* normalJsonPtr = InJson->GetObjectField(TEXT("normal")).Get();
		ret->NormalChannel = UG4MFMaterialChannel4D::FromJson(InG4MFState, ret, normalJsonPtr);
	}
	if (InJson->HasField(TEXT("orm")))
	{
		const FJsonObject* ormJsonPtr = InJson->GetObjectField(TEXT("orm")).Get();
		ret->OrmChannel = UG4MFMaterialChannel4D::FromJson(InG4MFState, ret, ormJsonPtr);
	}
	return ret;
}

TSharedPtr<FJsonObject> UG4MFMaterial4D::ToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (IsValid(BaseColorChannel))
	{
		const TSharedPtr<FJsonObject> baseColorJson = BaseColorChannel->ToJson();
		json->SetObjectField(TEXT("baseColor"), baseColorJson);
	}
	if (IsValid(EmissiveChannel))
	{
		const TSharedPtr<FJsonObject> emissiveJson = EmissiveChannel->ToJson();
		json->SetObjectField(TEXT("emissive"), emissiveJson);
	}
	if (IsValid(NormalChannel))
	{
		const TSharedPtr<FJsonObject> normalJson = NormalChannel->ToJson();
		json->SetObjectField(TEXT("normal"), normalJson);
	}
	if (IsValid(OrmChannel))
	{
		const TSharedPtr<FJsonObject> ormJson = OrmChannel->ToJson();
		json->SetObjectField(TEXT("orm"), ormJson);
	}
	return json;
}
