#pragma once

#include "../G4MFDefines.h"
#include "Templates/SharedPointer.h"

#include "G4MFItem4D.generated.h"

// UG4MFItem4D is a minimal class that handles common functionality for all G4MF items,
// including nodes and resources. It has a name, it can be converted to a Dictionary
// for serializing as JSON, and it can store additional data from extensions.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFItem4D : public UObject
{
	GENERATED_BODY()

private:
	// TODO: Store extras in an Unreal-serializable representation. FJsonObject is
	// intentionally kept for lossless G4MF round-tripping, but this field is not
	// preserved when a parsed UG4MFState4D is saved inside a .uasset.
	TSharedPtr<FJsonObject> _extras;

	UPROPERTY()
	FString _itemName;

public:
	FString GetItemName() const;
	void SetItemName(UObject* InG4MFState, const FString& InName);
	static FString SanitizeItemName(const FString& InName);

	void ReadItemEntriesFromJson(const FJsonObject* InJson);
	TSharedPtr<FJsonObject> WriteItemEntriesToJson() const;

	// Static helper functions.
	static TArray<TSharedPtr<FJsonValue>> ColorToJsonArray(const FLinearColor& InColor, const bool InForceIncludeAlpha = true);
	static FLinearColor JsonArrayToColor(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	static TArray<TSharedPtr<FJsonValue>> Int32ArrayToJsonArray(const TArray<int32>& InInt32Array);
	static TArray<int32> JsonArrayToInt32Array(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	static TArray<TSharedPtr<FJsonValue>> Vector3iToJsonArray(const FIntVector& InVector);
	static FIntVector JsonArrayToVector3i(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	static TArray<TSharedPtr<FJsonValue>> Vector3ToJsonArray(const FVector& InVector);
	static FVector JsonArrayToVector3(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
	// TODO: Delete this in favor of Vector4D's versions?
	static TArray<TSharedPtr<FJsonValue>> Vector4ToJsonArray(const FVector4& InVector);
	static FVector4 JsonArrayToVector4(const TArray<TSharedPtr<FJsonValue>>& InJsonArray);
};
