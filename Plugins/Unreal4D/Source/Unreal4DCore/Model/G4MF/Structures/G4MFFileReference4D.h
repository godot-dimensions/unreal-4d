#pragma once

#include "G4MFItem4D.h"

#include "G4MFFileReference4D.generated.h"

// UG4MFFileReference4D is used whenever G4MF files need to reference data that
// could be found in an external file, such as an image file, model file, audio file,
// or any other file. All file references are required to define the file's MIME type,
// and may refer to stored data either in a buffer view or an external file URI.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFFileReference4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int BufferViewIndex = -1;

	UPROPERTY()
	FString MimeType = TEXT("");

	UPROPERTY()
	FString FileUri = TEXT("");

	TArray64<uint8> ReadFileBytes(const UG4MFState4D* InG4MFState) const;
	bool CreateMissingDirectoriesIfNeeded(const UG4MFState4D* InG4MFState) const;
	bool WriteFileBytes(UG4MFState4D* InG4MFState, const TArray64<uint8>& InData, const int64 InAlignment = 4, const bool InDeduplicate = true, const int InBufferIndex = 0);

	virtual bool ImportParseFileData(UG4MFState4D* InG4MFState);
	virtual bool ExportSerializeFileData(UG4MFState4D* InG4MFState, const bool bInDeduplicate = true, const int32 InBufferIndex = 0);

	static UG4MFFileReference4D* FileReferenceFromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	void ReadFileReferenceEntriesFromJson(const FJsonObject* InJson);
	TSharedPtr<FJsonObject> WriteFileReferenceEntriesToJson() const;
};
