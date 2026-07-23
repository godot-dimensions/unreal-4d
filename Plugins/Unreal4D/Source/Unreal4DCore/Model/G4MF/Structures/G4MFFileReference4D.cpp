#include "G4MFFileReference4D.h"

#include "../G4MFState4D.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"

TArray64<uint8> UG4MFFileReference4D::ReadFileBytes(const UG4MFState4D* InG4MFState) const
{
	TArray64<uint8> ret;
	if (InG4MFState == nullptr) { return ret; }
	if (!FileUri.IsEmpty())
	{
		if (FileUri.StartsWith(TEXT("data:")))
		{
			const int32 base64Marker = FileUri.Find(TEXT(";base64,"));
			if (base64Marker == INDEX_NONE)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Only base64-encoded data URIs are currently supported."));
				return ret;
			}
			const FString payload = FileUri.Mid(base64Marker + 8);
			TArray<uint8> decodedData;
			if (!FBase64::Decode(payload, decodedData, EBase64Mode::Standard) &&
				!FBase64::Decode(payload, decodedData, EBase64Mode::UrlSafe))
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to decode base64 data URI."));
				return TArray64<uint8>();
			}
			ret.Append(decodedData.GetData(), decodedData.Num());
			return ret;
		}
		const FString path = FPaths::Combine(InG4MFState->G4MFBasePath, FileUri);
		if (IFileManager::Get().FileExists(*path))
		{
			// LoadFileToArray reserves two padding bytes after the data, but those
			// bytes are slack and are not included in the array's Num().
			if (!FFileHelper::LoadFileToArray(ret, *path) || ret.IsEmpty())
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to load data from the file at URI '%s', or the file was empty."), *path);
				return TArray64<uint8>();
			}
			return ret;
		}
		else
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to load file data from URI '%s'."), *path);
			return ret;
		}
	}
	if (BufferViewIndex >= 0)
	{
		const TArray<TObjectPtr<UG4MFBufferView4D>>& bufferViews = InG4MFState->G4MFBufferViews;
		if (bufferViews.IsValidIndex(BufferViewIndex))
		{
			const TObjectPtr<UG4MFBufferView4D>& bufferView = bufferViews[BufferViewIndex];
			if (bufferView != nullptr)
			{
				return bufferView->ReadBufferViewData(InG4MFState);
			}
		}
	}
	UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to load file data, file reference does not have a valid URI or buffer view index."));
	return ret;
}

bool UG4MFFileReference4D::CreateMissingDirectoriesIfNeeded(const UG4MFState4D* InG4MFState) const
{
	if (InG4MFState == nullptr) { return false; }
	if (FileUri.IsEmpty())
	{
		return true; // Nothing to create.
	}
	const FString relativePath = FPaths::GetPath(FileUri);
	if (relativePath.IsEmpty())
	{
		return true; // Nothing to create.
	}
	const FString fullPath = FPaths::Combine(InG4MFState->G4MFBasePath, relativePath);
	if (IFileManager::Get().DirectoryExists(*fullPath))
	{
		return true; // Nothing to create.
	}
	IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
	return platformFile.CreateDirectoryTree(*fullPath);
}

bool UG4MFFileReference4D::WriteFileBytes(UG4MFState4D* InG4MFState, const TArray64<uint8>& InData, const int64 InAlignment, const bool bInDeduplicate, const int InBufferIndex)
{
	if (InG4MFState == nullptr) { return false; }
	if (!FileUri.IsEmpty())
	{
		if (!CreateMissingDirectoriesIfNeeded(InG4MFState))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to create necessary directories for writing file at URI '%s'."), *FileUri);
			return false; // Failed to create necessary directories, so give up.
		}
		const FString filePath = FPaths::Combine(InG4MFState->G4MFBasePath, FileUri);
		if (!FFileHelper::SaveArrayToFile(InData, *filePath))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to write file data to URI '%s'."), *FileUri);
			return false;
		}
		return true;
	}
	// If there is no URI, write to a new buffer view.
	const int32 newBufferViewIndex = UG4MFBufferView4D::WriteNewBufferViewIntoState(InG4MFState, InData, InAlignment, bInDeduplicate, InBufferIndex);
	if (newBufferViewIndex < 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to write file data to a buffer view."));
		return false;
	}
	BufferViewIndex = newBufferViewIndex;
	return true;
}

bool UG4MFFileReference4D::ImportParseFileData(UG4MFState4D* InG4MFState)
{
	// This should be overridden by derived classes if they wish to add more complex logic during import.
	return true;
}

bool UG4MFFileReference4D::ExportSerializeFileData(UG4MFState4D* InG4MFState, const bool bInDeduplicate, const int32 InBufferIndex)
{
	// This should be overridden by derived classes if they wish to add more complex logic during export.
	return true;
}

UG4MFFileReference4D* UG4MFFileReference4D::FileReferenceFromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFFileReference4D* ret = NewObject<UG4MFFileReference4D>(InG4MFState, uniqueName);
	ret->ReadFileReferenceEntriesFromJson(InJson);
	return ret;
}

void UG4MFFileReference4D::ReadFileReferenceEntriesFromJson(const FJsonObject* InJson)
{
	if (InJson == nullptr) { return; }
	ReadItemEntriesFromJson(InJson);
	if (InJson->HasField(TEXT("bufferView")))
	{
		BufferViewIndex = (int)InJson->GetNumberField(TEXT("bufferView"));
	}
	if (InJson->HasField(TEXT("mimeType")))
	{
		MimeType = InJson->GetStringField(TEXT("mimeType"));
	}
	if (InJson->HasField(TEXT("uri")))
	{
		FileUri = InJson->GetStringField(TEXT("uri"));
	}
	if (MimeType.IsEmpty() && FileUri.StartsWith(TEXT("data:")))
	{
		const int32 mimeTypeEnd = FileUri.Find(TEXT(";"));
		if (mimeTypeEnd > 5)
		{
			MimeType = FileUri.Mid(5, mimeTypeEnd - 5);
		}
	}
}

TSharedPtr<FJsonObject> UG4MFFileReference4D::WriteFileReferenceEntriesToJson() const
{
	TSharedPtr<FJsonObject> json = WriteItemEntriesToJson();
	if (BufferViewIndex >= 0)
	{
		json->SetNumberField(TEXT("bufferView"), BufferViewIndex);
	}
	if (!MimeType.IsEmpty())
	{
		json->SetStringField(TEXT("mimeType"), MimeType);
	}
	if (!FileUri.IsEmpty())
	{
		json->SetStringField(TEXT("uri"), FileUri);
	}
	return json;
}
