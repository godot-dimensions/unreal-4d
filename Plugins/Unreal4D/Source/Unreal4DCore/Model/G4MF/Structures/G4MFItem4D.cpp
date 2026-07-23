#include "G4MFItem4D.h"

FString UG4MFItem4D::GetItemName() const
{
	return _itemName;
}

void UG4MFItem4D::SetItemName(UObject* InG4MFState, const FString& InName)
{
	_itemName = SanitizeItemName(InName);
	if (InG4MFState != nullptr && !_itemName.IsEmpty())
	{
		UObject* existingObject = FindObject<UObject>(InG4MFState, *_itemName);
		if (existingObject == nullptr || existingObject == this)
		{
			Rename(*_itemName, InG4MFState);
		}
	}
}

FString UG4MFItem4D::SanitizeItemName(const FString& InName)
{
	FString sanitizedName = InName;
	// Sanitize the name in-place by replacing characters which are invalid in Unreal asset names, Unreal package names, and G4MF item names.
	// Item names may be used for FNames and serializing to disk so we need to broadly disallow any of these.
	// Unreal's rules: See the "INVALID_*" macros in Engine/Source/Runtime/Core/Public/UObject/NameTypes.h.
	// G4MF's rules: All names in a G4MF file MUST NOT contain control characters or any of the
	// following characters: ", #, *, ., :, |, ?, @, <, >, {, }, [, ], /, literal \, and literal %.
	// Remove ASCII and Unicode control characters (0-31 and 127-159 inclusive). This includes `\n\r\t` which Unreal disallows.
	// Character codes 32 to 42 are ` !"#$%&'()*`, 44 is `,`, 46 is `.`, 47 is `/`, 58 to 64 are `:;<=>?@`,
	// 91 to 94 are `[\]^`, 96 is "`", and 123 to 126 are `{|}~`, which are all disallowed.
	// Allowed characters: 43 is `+`, 45 is `-`, 48 to 57 are `0-9`, 65 to 90 are `A-Z`, 95 is `_`, and 97 to 122 are `a-z`.
	// Additionally, any Unicode character at or above character code 160 is not disallowed by either Unreal or G4MF.
	for (int32 charIndex = 0; charIndex < sanitizedName.Len(); charIndex++)
	{
		uint32 charCode = sanitizedName[charIndex];
		if ((/*charCode >= 0 &&*/ charCode <= 42) ||
			charCode == 44 || charCode == 46 || charCode == 47 ||
			(charCode >= 58 && charCode <= 64) ||
			(charCode >= 91 && charCode <= 94) || charCode == 96 ||
			(charCode >= 123 && charCode <= 159))
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MFItem4D: The name '%s' contains a disallowed character '%c' (char code %d). Replacing with '_'."), *InName, charCode, charCode);
			sanitizedName[charIndex] = TEXT('_');
		}
	}
	return sanitizedName;
}

void UG4MFItem4D::ReadItemEntriesFromJson(const FJsonObject* InJson)
{
	// NOTE: Read the name from the UObject name, NOT from the JSON. The name MUST be read
	// before calling this function, because we read the name before constructing the object.
	// Unreal's `NewObject<T>` function accepts both an owner and a name as parameters.
	_itemName = GetName();
	if (InJson->HasField(TEXT("extras")))
	{
		_extras = InJson->GetObjectField(TEXT("extras"));
	}
}

TSharedPtr<FJsonObject> UG4MFItem4D::WriteItemEntriesToJson() const
{
	TSharedPtr<FJsonObject> json = MakeShared<FJsonObject>();
	if (_extras.IsValid())
	{
		json->SetObjectField(TEXT("extras"), _extras);
	}
	const FString name = GetItemName();
	if (!name.IsEmpty())
	{
		json->SetStringField(TEXT("name"), name);
	}
	return json;
}

// Static helper functions.

TArray<TSharedPtr<FJsonValue>> UG4MFItem4D::ColorToJsonArray(const FLinearColor& InColor, const bool bInForceIncludeAlpha)
{
	const bool bIncludeAlpha = bInForceIncludeAlpha || InColor.A != 1.0;
	TArray<TSharedPtr<FJsonValue>> jsonArray;
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InColor.R)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InColor.G)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InColor.B)));
	if (bIncludeAlpha)
	{
		jsonArray.Add(MakeShareable(new FJsonValueNumber(InColor.A)));
	}
	return jsonArray;
}

FLinearColor UG4MFItem4D::JsonArrayToColor(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	FLinearColor color = FLinearColor::White;
	if (InJsonArray.Num() >= 3)
	{
		color.R = InJsonArray[0]->AsNumber();
		color.G = InJsonArray[1]->AsNumber();
		color.B = InJsonArray[2]->AsNumber();
		if (InJsonArray.Num() >= 4)
		{
			color.A = InJsonArray[3]->AsNumber();
		}
	}
	else if (InJsonArray.Num() == 2)
	{
		color.R = InJsonArray[0]->AsNumber();
		color.G = InJsonArray[1]->AsNumber();
		color.B = 0.0f;
	}
	else if (InJsonArray.Num() == 1)
	{
		color.R = color.G = color.B = InJsonArray[0]->AsNumber();
	}
	return color;
}

// TODO: Delete this in favor of Vector4D's versions?
TArray<TSharedPtr<FJsonValue>> UG4MFItem4D::Vector4ToJsonArray(const FVector4& InVector)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.X)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Y)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Z)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.W)));
	return jsonArray;
}

FVector4 UG4MFItem4D::JsonArrayToVector4(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	FVector4 vec = FVector4(0.0, 0.0, 0.0, 0.0);
	if (InJsonArray.Num() >= 4)
	{
		vec.X = InJsonArray[0]->AsNumber();
		vec.Y = InJsonArray[1]->AsNumber();
		vec.Z = InJsonArray[2]->AsNumber();
		vec.W = InJsonArray[3]->AsNumber();
	}
	return vec;
}

TArray<TSharedPtr<FJsonValue>> UG4MFItem4D::Int32ArrayToJsonArray(const TArray<int32>& InInt32Array)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;
	for (int32 Value : InInt32Array)
	{
		jsonArray.Add(MakeShareable(new FJsonValueNumber(Value)));
	}
	return jsonArray;
}

TArray<int32> UG4MFItem4D::JsonArrayToInt32Array(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	TArray<int32> Int32Array;
	for (const auto& JsonValue : InJsonArray)
	{
		Int32Array.Add((int32)JsonValue->AsNumber());
	}
	return Int32Array;
}

TArray<TSharedPtr<FJsonValue>> UG4MFItem4D::Vector3iToJsonArray(const FIntVector& InVector)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.X)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Y)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Z)));
	return jsonArray;
}

FIntVector UG4MFItem4D::JsonArrayToVector3i(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	FIntVector vec = FIntVector(0, 0, 0);
	if (InJsonArray.Num() >= 3)
	{
		vec.X = (int32)InJsonArray[0]->AsNumber();
		vec.Y = (int32)InJsonArray[1]->AsNumber();
		vec.Z = (int32)InJsonArray[2]->AsNumber();
	}
	else if (InJsonArray.Num() >= 1)
	{
		vec.X = vec.Y = vec.Z = (int32)InJsonArray[0]->AsNumber();
	}
	return vec;
}

TArray<TSharedPtr<FJsonValue>> UG4MFItem4D::Vector3ToJsonArray(const FVector& InVector)
{
	TArray<TSharedPtr<FJsonValue>> jsonArray;
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.X)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Y)));
	jsonArray.Add(MakeShareable(new FJsonValueNumber(InVector.Z)));
	return jsonArray;
}

FVector UG4MFItem4D::JsonArrayToVector3(const TArray<TSharedPtr<FJsonValue>>& InJsonArray)
{
	FVector vec = FVector(0.0, 0.0, 0.0);
	if (InJsonArray.Num() >= 3)
	{
		vec.X = InJsonArray[0]->AsNumber();
		vec.Y = InJsonArray[1]->AsNumber();
		vec.Z = InJsonArray[2]->AsNumber();
	}
	else if (InJsonArray.Num() >= 1)
	{
		vec.X = vec.Y = vec.Z = InJsonArray[0]->AsNumber();
	}
	return vec;
}
