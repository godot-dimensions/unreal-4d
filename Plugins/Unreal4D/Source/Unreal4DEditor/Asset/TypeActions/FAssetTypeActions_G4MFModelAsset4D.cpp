#include "FAssetTypeActions_G4MFModelAsset4D.h"

#include "../../../Unreal4DCore/Model/Asset/G4MFModelAsset4D.h"

#include "EditorFramework/AssetImportData.h"

UClass* FAssetTypeActions_G4MFModelAsset4D::GetSupportedClass() const
{
	return UG4MFModelAsset4D::StaticClass();
}

void FAssetTypeActions_G4MFModelAsset4D::GetResolvedSourceFilePaths(
	const TArray<UObject*>& TypeAssets,
	TArray<FString>& OutSourceFilePaths) const
{
	for (const UObject* Asset : TypeAssets)
	{
		const UModelAsset4D* ModelAsset = Cast<UModelAsset4D>(Asset);
		if (ModelAsset && ModelAsset->AssetImportData)
		{
			TArray<FString> ExtractedFilenames;
			ModelAsset->AssetImportData->ExtractFilenames(ExtractedFilenames);
			if (ExtractedFilenames.Num() > 0)
			{
				OutSourceFilePaths.Append(ExtractedFilenames);
			}
		}
	}
}
