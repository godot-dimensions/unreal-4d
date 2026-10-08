#pragma once

#include "../../Unreal4DEditorDefines.h"

#include "Factories/Factory.h"
#include "EditorReimportHandler.h"

#include "G4MFModelAsset4DFactory.generated.h"

// Import factory for G4MF 4D model files (.g4tf / .g4b).
// Handles first-time import and reimport of G4MF files into
// UG4MFModelAsset4D .uasset files.
//
// Also implements FReimportHandler so the same class drives reimport
// (keeps logic in one place and mirrors how UFontFileImportFactory works).
//
// TODO: The behavior is a bit inconsistent, sometimes Unreal doesn't
// detect changes to the source file or it fails to trigger reimport.
// Further research could be done into the details of Unreal's asset system.
UCLASS()
class UG4MFModelAsset4DFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()

public:
	UG4MFModelAsset4DFactory();

	// UFactory overrides.
	virtual bool FactoryCanImport(const FString& InFilename) override;
	virtual UObject* FactoryCreateFile(
		UClass* InClass,
		UObject* InParent,
		FName InName,
		EObjectFlags InFlags,
		const FString& InAbsFilePath,
		const TCHAR* InParms,
		FFeedbackContext* InWarn,
		bool& bOutOperationCanceled
	) override;

	// FReimportHandler overrides.
	virtual bool CanReimport(UObject* InObject, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* InObject, const TArray<FString>& InNewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* InObject) override;

	// Link this FReimportHandler back to its owning UFactory object for garbage collection and reimport routing.
	virtual TObjectPtr<UObject>* GetFactoryObject() const override
	{
		return &GCMark;
	}

	mutable TObjectPtr<UObject> GCMark{this};

private:
	// Shared import logic used by both first-time import and reimport.
	// Returns the created/updated asset, or nullptr on failure.
	class UG4MFModelAsset4D* ImportG4MFFile(
		UObject* InParent,
		FName InName,
		EObjectFlags InFlags,
		const FString& InAbsFilePath,
		FFeedbackContext* InWarn
	);
};
