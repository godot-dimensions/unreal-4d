#pragma once

#include "../../Unreal4DEditorDefines.h"
#include "AssetTypeActions_Base.h"

// Defines how UG4MFModelAsset4D appears in the Content Browser:
// name, color, category, and supported class.
class FAssetTypeActions_G4MFModelAsset4D : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions interface.
	virtual FText GetName() const override
	{
		return FText::FromString(TEXT("G4MF Model 4D"));
	}

	virtual FColor GetTypeColor() const override
	{
		return FColor(230, 191, 25);
	}

	virtual UClass* GetSupportedClass() const override;

	virtual uint32 GetCategories() override
	{
		// TODO: Register a custom "4D" asset category if desired.
		return EAssetTypeCategories::Basic;
	}

	// Tell the Content Browser this asset comes from import so it shows "Reimport" on right-click.
	virtual bool IsImportedAsset() const override
	{
		return true;
	}

	// Provide the source file paths so the engine can track them for auto-reimport.
	virtual void GetResolvedSourceFilePaths(
		const TArray<UObject*>& TypeAssets,
		TArray<FString>& OutSourceFilePaths) const override;
};
