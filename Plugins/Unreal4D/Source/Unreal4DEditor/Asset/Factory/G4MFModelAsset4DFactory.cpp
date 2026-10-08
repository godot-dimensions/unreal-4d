#include "G4MFModelAsset4DFactory.h"

#include "../../../Unreal4DCore/Model/Asset/G4MFModelAsset4D.h"
#include "../../../Unreal4DCore/Model/G4MF/G4MFDocument4D.h"
#include "../../../Unreal4DCore/Model/G4MF/G4MFState4D.h"
#include "../../../Unreal4DCore/Model/Mesh/Tetra/ArrayTetraMesh4D.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/Paths.h"
#include "ObjectTools.h"

UG4MFModelAsset4DFactory::UG4MFModelAsset4DFactory()
{
	bEditorImport = true;
	bCreateNew = false;
	bText = false;

	SupportedClass = UG4MFModelAsset4D::StaticClass();

	Formats.Add(TEXT("g4tf;G4MF Text Format"));
	Formats.Add(TEXT("g4b;G4MF Binary Format"));
}

// UFactory.

bool UG4MFModelAsset4DFactory::FactoryCanImport(const FString& InFilename)
{
	const FString extension = FPaths::GetExtension(InFilename).ToLower();
	return extension == TEXT("g4tf") || extension == TEXT("g4b");
}

UObject* UG4MFModelAsset4DFactory::FactoryCreateFile(
	UClass* InClass,
	UObject* InParent,
	FName InName,
	EObjectFlags InFlags,
	const FString& InAbsFilePath,
	const TCHAR* InParms,
	FFeedbackContext* InWarn,
	bool& bOutOperationCanceled)
{
	bOutOperationCanceled = false;
	return ImportG4MFFile(InParent, InName, InFlags, InAbsFilePath, InWarn);
}

// FReimportHandler.

bool UG4MFModelAsset4DFactory::CanReimport(UObject* InObject, TArray<FString>& OutFilenames)
{
	UG4MFModelAsset4D* asset = Cast<UG4MFModelAsset4D>(InObject);
	if (!asset || !asset->AssetImportData)
	{
		return false;
	}

	asset->AssetImportData->ExtractFilenames(OutFilenames);
	return OutFilenames.Num() > 0;
}

void UG4MFModelAsset4DFactory::SetReimportPaths(UObject* InObject, const TArray<FString>& InNewReimportPaths)
{
	UG4MFModelAsset4D* asset = Cast<UG4MFModelAsset4D>(InObject);
	if (asset && asset->AssetImportData && InNewReimportPaths.Num() > 0)
	{
		asset->AssetImportData->UpdateFilenameOnly(InNewReimportPaths[0]);
	}
}

EReimportResult::Type UG4MFModelAsset4DFactory::Reimport(UObject* InObject)
{
	UG4MFModelAsset4D* asset = Cast<UG4MFModelAsset4D>(InObject);
	if (!asset || !asset->AssetImportData)
	{
		return EReimportResult::Failed;
	}

	const FString sourcePath = asset->AssetImportData->GetFirstFilename();
	if (sourcePath.IsEmpty() || !FPaths::FileExists(sourcePath))
	{
		UE_LOG(LogUnreal4DEditor, Warning,
			TEXT("G4MF reimport: Source file not found '%s'. Keeping existing embedded state."),
			*sourcePath);
		return EReimportResult::Failed;
	}

	// Re-run the import into the existing asset's outer.
	UG4MFModelAsset4D* updated = ImportG4MFFile(
		asset->GetOuter(),
		asset->GetFName(),
		asset->GetFlags(),
		sourcePath,
		GWarn
	);

	if (updated == nullptr)
	{
		return EReimportResult::Failed;
	}

	// If ImportG4MFFile returned a different object (shouldn't for reimport
	// when the outer matches), copy the data over.
	if (updated != asset)
	{
		asset->EmbeddedState = updated->EmbeddedState;
		asset->ImportDocument = updated->ImportDocument;
		asset->ImportVersion = updated->ImportVersion;
		asset->AssetImportData = updated->AssetImportData;
	}

	return EReimportResult::Succeeded;
}

// Shared import logic.

UG4MFModelAsset4D* UG4MFModelAsset4DFactory::ImportG4MFFile(
	UObject* InParent,
	FName InName,
	EObjectFlags InFlags,
	const FString& InAbsFilePath,
	FFeedbackContext* InWarn)
{
	// 1. Create (or find existing) the model asset object.
	const FString nameString = ObjectTools::SanitizeInvalidChars(InName.ToString(), INVALID_LONGPACKAGE_CHARACTERS);
	UG4MFModelAsset4D* modelAsset = FindObject<UG4MFModelAsset4D>(InParent, *nameString);
	if (modelAsset == nullptr)
	{
		modelAsset = NewObject<UG4MFModelAsset4D>(InParent, UG4MFModelAsset4D::StaticClass(), FName(nameString), InFlags);
	}
	else
	{
		// Existing model asset, clear its existing data before reimporting new data into it.
		modelAsset->Modify();
		modelAsset->EmbeddedState = nullptr;
		modelAsset->ImportDocument = nullptr;
	}

	// 2. Create document + state owned by the model asset.
	UG4MFDocument4D* g4mfDocument = NewObject<UG4MFDocument4D>(modelAsset, TEXT("ImportDocument"));
	UG4MFState4D* g4mfState = NewObject<UG4MFState4D>(modelAsset, TEXT("EmbeddedState"));

	// 3. Run the document import from the file.
	const bool bSuccess = g4mfDocument->ImportReadFromFile(g4mfState, InAbsFilePath);
	if (!bSuccess)
	{
		UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: ImportReadFromFile failed for '%s'."), *InAbsFilePath);
		return nullptr;
	}

	// 4. Convert coordinate system from G4MF's right-handed meters to Unreal's left-handed centimeters.
	const FMatrix g4mfToUnrealBasis = UBasis4D::Basis4D_FromNumbers(
		0, -1, 0, 0, // G4MF +X -> Unreal -Y (sideways)
		0, 0, +1, 0, // G4MF +Y -> Unreal +Z (up)
		+1, 0, 0, 0, // G4MF +Z -> Unreal +X (forward)
		0, 0, 0, +1 // G4MF +W -> Unreal +W (unchanged)
	);
	constexpr double g4mfToUnrealScale = 100.0; // G4MF meters -> Unreal centimeters
	g4mfDocument->ConvertCoordinateSystem(g4mfState, g4mfToUnrealBasis, g4mfToUnrealScale);

	// 5. Assign the results into the model asset.
	modelAsset->EmbeddedState = g4mfState;
	modelAsset->ImportDocument = g4mfDocument;
	modelAsset->ImportVersion++;

	// 6. Record source file for reimport.
	if (modelAsset->AssetImportData == nullptr)
	{
		modelAsset->AssetImportData = NewObject<UAssetImportData>(modelAsset, TEXT("AssetImportData"));
	}
	modelAsset->AssetImportData->Update(InAbsFilePath);

	// 7. Get the model asset's package and prepare the asset for holding the imported mesh .uasset files.
	// The modelPackagePath includes the project-relative path and the file name excluding the file extension.
	const FString modelPackagePath = InParent->GetOutermost()->GetName();
	UPackage* modelPackage = modelAsset->GetOutermost();

	// 8. Generate individual meshes and save them to separate .uasset files, so they can be individually referenced.
	constexpr EObjectFlags standaloneFlags = EObjectFlags(RF_Public | RF_Standalone);
	const int32 meshCount = g4mfState->G4MFMeshes.Num();
	g4mfState->UnrealMeshAssets.SetNum(meshCount);
	for (int32 meshIndex = 0; meshIndex < meshCount; meshIndex++)
	{
		UG4MFMesh4D* g4mfMesh = g4mfState->G4MFMeshes[meshIndex];
		FString meshAssetName = g4mfMesh->GetItemName();
		if (meshAssetName.IsEmpty())
		{
			// For example, "filename.g4b" with an unnamed mesh would produce a mesh asset named "filename_Mesh0.uasset".
			meshAssetName = FString::Printf(TEXT("%s_Mesh%d"), *nameString, meshIndex);
		}
		else
		{
			// For example, "filename.g4b" with a mesh named "Body" would produce a mesh asset named "filename_Body.uasset".
			meshAssetName = FString::Printf(TEXT("%s_%s"), *nameString, *meshAssetName);
		}
		const FString meshPackagePath = modelPackagePath / meshAssetName;
		UPackage* meshPackage = CreatePackage(*meshPackagePath);
		// Ideally, it would be nice to completely replace the old object with the new object.
		// However, Unreal's reimport system expects the old object to be updated in place,
		// so we have to find the old object and update it instead of just using a new object.
		UMesh4D* meshAsset = FindObject<UMesh4D>(meshPackage, *meshAssetName);
		if (meshAsset == nullptr)
		{
			meshAsset = g4mfMesh->ImportGenerateMesh(g4mfState, meshPackage, standaloneFlags);
			if (meshAsset == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Failed to generate mesh asset for mesh '%s'. Aborting file import."), *meshAssetName);
				return nullptr;
			}
			if (meshAsset->GetFName() != FName(*meshAssetName))
			{
				meshAsset->Rename(*meshAssetName, meshPackage);
			}
			FAssetRegistryModule::AssetCreated(meshAsset);
		}
		else
		{
			// Existing mesh asset, overwrite its data.
			meshAsset->Modify();
			UArrayTetraMesh4D* existingArrayTetraMesh = Cast<UArrayTetraMesh4D>(meshAsset);
			if (existingArrayTetraMesh == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Cannot update existing mesh asset '%s' because it is not a UArrayTetraMesh4D. Aborting file import."), *meshAssetName);
				return nullptr;
			}
			UMesh4D* tempMesh = g4mfMesh->ImportGenerateMesh(g4mfState, GetTransientPackage(), RF_Transient);
			if (tempMesh == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Failed to generate temporary mesh for mesh '%s' during asset update. Aborting file import."), *meshAssetName);
				return nullptr;
			}
			existingArrayTetraMesh->OverwriteFromOtherMesh(tempMesh);
		}
		meshAsset->MarkPackageDirty();
		meshAsset->ClearFlags(RF_Transient);
		meshAsset->SetFlags(standaloneFlags);
		meshPackage->MarkPackageDirty();
		g4mfState->UnrealMeshAssets[meshIndex] = meshAsset;
	}

	// 9. If there are multiple meshes, or multiple instances of meshes, also generate a combined mesh for the whole model.
	if (g4mfState->G4MFMeshes.Num() > 1 || g4mfState->G4MFNodes.Num() > 1)
	{
		// For example, "filename.g4b" with multiple meshes would produce a combined mesh asset named "filename_CombinedMesh.uasset".
		const FString combinedMeshAssetName = FString::Printf(TEXT("%s_CombinedMesh"), *nameString);
		const FString combinedMeshPackagePath = modelPackagePath / combinedMeshAssetName;
		UPackage* combinedMeshPackage = CreatePackage(*combinedMeshPackagePath);
		constexpr bool bIncludeInvisible = false;
		// Ideally, it would be nice to completely replace the old object with the new object.
		// However, Unreal's reimport system expects the old object to be updated in place,
		// so we have to find the old object and update it instead of just using a new object.
		UMesh4D* combinedMeshAsset = FindObject<UMesh4D>(combinedMeshPackage, *combinedMeshAssetName);
		if (combinedMeshAsset == nullptr)
		{
			combinedMeshAsset = g4mfDocument->ImportGenerateSingleMesh(g4mfState, combinedMeshPackage, standaloneFlags, -1, bIncludeInvisible);
			if (combinedMeshAsset == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Failed to generate combined mesh asset. Aborting file import."));
				return nullptr;
			}
			if (combinedMeshAsset->GetFName() != FName(*combinedMeshAssetName))
			{
				combinedMeshAsset->Rename(*combinedMeshAssetName, combinedMeshPackage);
			}
			FAssetRegistryModule::AssetCreated(combinedMeshAsset);
		}
		else
		{
			// Existing mesh asset, overwrite its data.
			combinedMeshAsset->Modify();
			UArrayTetraMesh4D* existingCombinedArrayTetraMesh = Cast<UArrayTetraMesh4D>(combinedMeshAsset);
			if (existingCombinedArrayTetraMesh == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Cannot update existing combined mesh asset '%s' because it is not a UArrayTetraMesh4D. Aborting file import."), *combinedMeshAssetName);
				return nullptr;
			}
			UMesh4D* tempCombinedMesh = g4mfDocument->ImportGenerateSingleMesh(g4mfState, GetTransientPackage(), RF_Transient, -1, bIncludeInvisible);
			if (tempCombinedMesh == nullptr)
			{
				UE_LOG(LogUnreal4DEditor, Error, TEXT("G4MF import: Failed to generate temporary combined mesh during asset update. Aborting file import."), *combinedMeshAssetName);
				return nullptr;
			}
			existingCombinedArrayTetraMesh->OverwriteFromOtherMesh(tempCombinedMesh);
		}
		g4mfState->UnrealCombinedMeshAsset = combinedMeshAsset;
		g4mfState->bUnrealCombinedMeshAssetIncludeInvisibleSetting = bIncludeInvisible;
		combinedMeshAsset->ClearFlags(RF_Transient);
		combinedMeshAsset->SetFlags(standaloneFlags);
		combinedMeshAsset->MarkPackageDirty();
		combinedMeshPackage->MarkPackageDirty();
	}

	// 10. Mark the main model asset dirty to prompt user to save it.
	modelAsset->MarkPackageDirty();
	modelPackage->MarkPackageDirty();

	// TODO: If additional import settings (compression, etc.) should be
	// exposed to users via an import options dialog, hook that up here.

	UE_LOG(LogUnreal4DEditor, Display, TEXT("G4MF import: Successfully imported '%s' into model '%s' with %d meshes stored separately in a subfolder."), *InAbsFilePath, *modelAsset->GetName(), g4mfState->UnrealMeshAssets.Num());
	return modelAsset;
}
