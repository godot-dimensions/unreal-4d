#include "ModelAsset4D.h"

void UModelAsset4D::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITORONLY_DATA
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}
#endif
}

void UModelAsset4D::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
#if WITH_EDITORONLY_DATA
	if (AssetImportData)
	{
		Context.AddTag(FAssetRegistryTag(
			SourceFileTagName(),
			AssetImportData->GetSourceData().ToJson(),
			FAssetRegistryTag::TT_Hidden
		));
	}
#endif
	Super::GetAssetRegistryTags(Context);
}

UScene4DComponent* UModelAsset4D::GenerateComponentHierarchyForActor(AActor* InActor)
{
	// Base implementation returns nullptr.
	// Subclasses override to generate format-specific component hierarchies.
	return nullptr;
}

FString UModelAsset4D::GetSourceFormatName() const
{
	return TEXT("Unknown");
}
