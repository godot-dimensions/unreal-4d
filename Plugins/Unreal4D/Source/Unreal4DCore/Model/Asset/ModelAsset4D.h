#pragma once

#include "../../Unreal4DDefines.h"

#include "EditorFramework/AssetImportData.h"

#include "ModelAsset4D.generated.h"

class UScene4DComponent;

// Base class for imported 4D model assets.
//
// Represents a fully imported 4D model stored as a .uasset file.
// Subclasses provide format-specific data (such as G4MF).
//
// Design: this is a self-contained runtime asset that embeds all data
// needed for packaged builds without depending on the original source file.
UCLASS(BlueprintType, Category = "Unreal 4D|Model")
class UNREAL4DCORE_API UModelAsset4D : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITORONLY_DATA
	// Tracks the source file for reimport support.
	UPROPERTY(VisibleAnywhere, Category = "Import")
	TObjectPtr<UAssetImportData> AssetImportData;
#endif

	// Version counter incremented on each reimport.
	UPROPERTY(VisibleAnywhere, Category = "Import")
	int32 ImportVersion = 0;

	// UObject interface.
	virtual void PostInitProperties() override;
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;

	// Generate a full component hierarchy from the embedded model data.
	// Needs to have an actor created ahead of time to own the components.
	virtual UScene4DComponent* GenerateComponentHierarchyForActor(AActor* InActor);

	// Human-readable description of the source format.
	virtual FString GetSourceFormatName() const;
};
