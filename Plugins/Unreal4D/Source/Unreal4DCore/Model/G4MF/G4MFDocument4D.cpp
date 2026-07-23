#include "G4MFDocument4D.h"

#include "../../Scene/Components/Scene4DComponent.h"
#include "../../Scene/Components/Camera4DComponent.h"
#include "../../Scene/Components/Mesh4DComponent.h"
#include "../Mesh/Tetra/ArrayTetraMesh4D.h"
#include "Containers/StringConv.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Base64.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

// Byte array helper functions (abstractions using `FMemory::Memcpy` for cross-platform safety).

uint32 UG4MFDocument4D::ByteArrayReadUint32(const uint8* InByteArrayPtr, const uint64 InOffset)
{
	uint32 value;
	FMemory::Memcpy(&value, InByteArrayPtr + InOffset, sizeof(value));
	return value;
}

uint64 UG4MFDocument4D::ByteArrayReadUint64(const uint8* InByteArrayPtr, const uint64 InOffset)
{
	uint64 value;
	FMemory::Memcpy(&value, InByteArrayPtr + InOffset, sizeof(value));
	return value;
}

// File helper functions (abstractions on top of limited Unreal IFileHandle API).

uint32 UG4MFDocument4D::FileReadUint32(const TUniquePtr<IFileHandle>& InFileHandle)
{
	uint32 Value = 0;
	const bool success = InFileHandle->Read(reinterpret_cast<uint8*>(&Value), sizeof(uint32));
	ensure(success);
	return Value;
}

uint64 UG4MFDocument4D::FileReadUint64(const TUniquePtr<IFileHandle>& InFileHandle)
{
	uint64 Value = 0;
	const bool success = InFileHandle->Read(reinterpret_cast<uint8*>(&Value), sizeof(uint64));
	ensure(success);
	return Value;
}

EG4MFEncodingFormat4D UG4MFDocument4D::ValidateEncodingFormat(const uint32 InEncodingFormatUint32) {
	if (InEncodingFormatUint32 == (uint32)EG4MFEncodingFormat4D::Plain)
	{
		return EG4MFEncodingFormat4D::Plain;
	}
	if (InEncodingFormatUint32 == (uint32)EG4MFEncodingFormat4D::Zstd)
	{
		return EG4MFEncodingFormat4D::Zstd;
	}
	return EG4MFEncodingFormat4D::Unknown;
}

bool UG4MFDocument4D::IsEncodingFormatSupported(const EG4MFEncodingFormat4D InEncodingFormat) {
	return InEncodingFormat == EG4MFEncodingFormat4D::Plain;
}

FString UG4MFDocument4D::Uint32ToAsciiString(uint32 InValue, const bool bInAllowAndEscapeNonAscii)
{
	FString str = TEXT("");
	for (int i = 0; i < 4; ++i)
	{
		const uint8 LowByte = (uint8)InValue;
		if (LowByte > 0x1F && LowByte < 0x7F)
		{
			str += (TCHAR)LowByte;
		} else if (bInAllowAndEscapeNonAscii)
		{
			str += FString::Printf(TEXT("\\u00%02X"), LowByte);
		}
		else
		{
			str += TEXT("?");
		}
		InValue >>= 8;
	}
	return str;
}

uint32 UG4MFDocument4D::AsciiStringToUint32(const FString& InValue)
{
	if (InValue.Len() != 4)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: String to uint32 conversion expects a 4-character string."));
		return 0;
	}
	uint32 Value = 0;
	for (int i = 0; i < 4; ++i)
	{
		const uint8 LowByte = (uint8)InValue[i];
		Value |= LowByte << (i * 8);
	}
	return Value;
}

bool UG4MFDocument4D::ExportConvertSceneComponent(UG4MFState4D* InG4MFState, UScene4DComponent* InComponent, const int32 InParentIndex)
{
	if (InG4MFState == nullptr || InComponent == nullptr) { return false; }
	UG4MFNode4D* node = UG4MFNode4D::FromSceneComponentBasic(InG4MFState, InComponent);
	// The root node MUST NOT have a transform as required by the G4MF specification.
	if (InParentIndex == -1)
	{
		node->Transform = FTransform4D::Identity();
		node->bIsVisible = true;
	}
	else
	{
		node->ParentIndex = InParentIndex;
	}
	// Append the node. This is always guaranteed to be at the end of the array.
	const int32 nodeIndex = InG4MFState->AppendG4MFNode(node);
	InG4MFState->UnrealComponents.SetNum(nodeIndex + 1);
	InG4MFState->UnrealComponents[nodeIndex] = InComponent;
	// Handle the functional parts of this component and its children, if they exist.
	node->ExportConvertFunctionalComponents(InG4MFState, InComponent);
	// Recurse into 4D children.
	for (USceneComponent* child : InComponent->GetAttachChildren())
	{
		UScene4DComponent* child4D = Cast<UScene4DComponent>(child);
		if (child4D == nullptr)
		{
			continue;
		}
		const int32 childIndexIfAppended = InG4MFState->G4MFNodes.Num();
		const bool success = ExportConvertSceneComponent(InG4MFState, child4D, nodeIndex);
		if (!success)
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF export: Failed to convert scene component '%s' and its children into G4MF nodes."), *child->GetName());
			return false;
		}
		node->ChildrenIndices.Add(childIndexIfAppended);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeJsonData(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return false; }
	InG4MFState->G4MFJson = MakeShared<FJsonObject>();
	FJsonObject* g4mfJsonRawPtr = InG4MFState->G4MFJson.Get();
	bool success = ExportSerializeNodes(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF nodes into JSON."));
		return false;
	}
	success = ExportSerializeShapes(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF shapes into JSON."));
		return false;
	}
	success = ExportSerializeMeshes(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF meshes into JSON."));
		return false;
	}
	success = ExportSerializeMaterials(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF materials into JSON."));
		return false;
	}
	success = ExportSerializeTextures(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF textures into JSON."));
		return false;
	}
	success = ExportSerializeFiles(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF files into JSON."));
		return false;
	}
	success = ExportSerializeAssetHeader(InG4MFState, g4mfJsonRawPtr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF asset header into JSON."));
		return false;
	}
	// Serialize buffers, buffers views, and accessors last, in case any of the above added new ones.
	success = ExportSerializeBuffersAccessors(InG4MFState, g4mfJsonRawPtr);
	return success;
}

bool UG4MFDocument4D::ExportSerializeNodes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	TArray<TSharedPtr<FJsonValue>> serializedNodes;
	for (int nodeIndex = 0; nodeIndex < InG4MFState->G4MFNodes.Num(); nodeIndex++)
	{
		const TObjectPtr<UG4MFNode4D>& g4mfNode = InG4MFState->G4MFNodes[nodeIndex];
		if (IsValid(g4mfNode))
		{
			if (nodeIndex == 0)
			{
				// The root node's transform MUST be the identity transform, as per the specification.
				g4mfNode->Transform = FTransform4D::Identity();
			}
			TSharedPtr<FJsonObject> nodeJson = g4mfNode->ToJson();
			serializedNodes.Add(MakeShared<FJsonValueObject>(nodeJson));
		}
	}
	if (!serializedNodes.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("nodes"), serializedNodes);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeShapes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (InG4MFState->G4MFShapes.IsEmpty())
	{
		return true; // No shapes to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedShapes;
	for (const TObjectPtr<UG4MFShape4D>& g4mfShape : InG4MFState->G4MFShapes)
	{
		if (IsValid(g4mfShape))
		{
			TSharedPtr<FJsonObject> shapeJson = g4mfShape->ToJson();
			serializedShapes.Add(MakeShared<FJsonValueObject>(shapeJson));
		}
	}
	if (!serializedShapes.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("shapes"), serializedShapes);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeMeshes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (InG4MFState->G4MFMeshes.IsEmpty())
	{
		return true; // No meshes to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedMeshes;
	for (const TObjectPtr<UG4MFMesh4D>& g4mfMesh : InG4MFState->G4MFMeshes)
	{
		if (IsValid(g4mfMesh))
		{
			TSharedPtr<FJsonObject> MeshJson = g4mfMesh->ToJson();
			serializedMeshes.Add(MakeShared<FJsonValueObject>(MeshJson));
		}
	}
	if (!serializedMeshes.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("meshes"), serializedMeshes);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeMaterials(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (InG4MFState->G4MFMaterials.IsEmpty())
	{
		return true; // No materials to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedMaterials;
	for (const TObjectPtr<UG4MFMaterial4D>& g4mfMaterial : InG4MFState->G4MFMaterials)
	{
		if (IsValid(g4mfMaterial))
		{
			TSharedPtr<FJsonObject> materialJson = g4mfMaterial->ToJson();
			serializedMaterials.Add(MakeShared<FJsonValueObject>(materialJson));
		}
	}
	if (!serializedMaterials.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("materials"), serializedMaterials);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeTextures(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (InG4MFState->G4MFTextures.IsEmpty())
	{
		return true; // No textures to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedTextures;
	for (const TObjectPtr<UG4MFTexture4D>& g4mfTexture : InG4MFState->G4MFTextures)
	{
		if (IsValid(g4mfTexture))
		{
			TSharedPtr<FJsonObject> textureJson = g4mfTexture->ToJson();
			serializedTextures.Add(MakeShared<FJsonValueObject>(textureJson));
		}
	}
	if (!serializedTextures.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("textures"), serializedTextures);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeFiles(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (InG4MFState->G4MFFiles.IsEmpty())
	{
		return true; // No files to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedFiles;
	for (const TObjectPtr<UG4MFFileReference4D>& g4mfFile : InG4MFState->G4MFFiles)
	{
		if (IsValid(g4mfFile))
		{
			const bool exportFileSuccess = g4mfFile->ExportSerializeFileData(InG4MFState);
			if (!exportFileSuccess)
			{
				return false;
			}
			TSharedPtr<FJsonObject> fileJson = g4mfFile->WriteFileReferenceEntriesToJson();
			serializedFiles.Add(MakeShared<FJsonValueObject>(fileJson));
		}
	}
	if (!serializedFiles.IsEmpty())
	{
		InG4MFJson->SetArrayField(TEXT("files"), serializedFiles);
	}
	return true;
}

bool UG4MFDocument4D::ExportSerializeAssetHeader(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	TSharedPtr<FJsonObject> assetHeader = MakeShared<FJsonObject>();
	assetHeader->SetNumberField(TEXT("dimension"), 4);
	assetHeader->SetStringField(TEXT("generator"), TEXT("Unreal Engine with Unreal 4D plugin"));
	assetHeader->SetStringField(TEXT("specification"), TEXT("https://github.com/godot-dimensions/g4mf"));
	InG4MFJson->SetObjectField(TEXT("asset"), assetHeader);
	return true;
}

bool UG4MFDocument4D::ExportSerializeBuffersAccessors(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	// Serialize buffers.
	const int32 bufferCount = InG4MFState->G4MFBuffers.Num();
	if (bufferCount == 0)
	{
		return true; // No buffers to serialize (implies no buffer views or accessors).
	}
	TArray<TSharedPtr<FJsonValue>> serializedBuffers;
	for (int32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		const FG4MFBuffer4D& stateBuffer = InG4MFState->G4MFBuffers[bufferIndex];
		TSharedPtr<FJsonObject> bufferJson = MakeShared<FJsonObject>();
		bufferJson->SetNumberField(TEXT("byteLength"), stateBuffer.Data.Num());
		if (EncodingFormat != EG4MFEncodingFormat4D::Plain)
		{
			bufferJson->SetStringField(TEXT("encoding"), Uint32ToAsciiString((uint32)EncodingFormat, true));
		}
		serializedBuffers.Add(MakeShared<FJsonValueObject>(bufferJson));
	}
	InG4MFJson->SetArrayField(TEXT("buffers"), serializedBuffers);
	// Serialize buffer views (only relevant if buffers exist).
	const int32 bufferViewCount = InG4MFState->G4MFBufferViews.Num();
	if (bufferViewCount == 0)
	{
		return true; // No buffer views to serialize (implies no accessors).
	}
	TArray<TSharedPtr<FJsonValue>> serializedBufferViews;
	for (int32 bufferViewIndex = 0; bufferViewIndex < bufferViewCount; bufferViewIndex++)
	{
		const TObjectPtr<UG4MFBufferView4D>& stateBufferView = InG4MFState->G4MFBufferViews[bufferViewIndex];
		if (!IsValid(stateBufferView))
		{
			return false;
		}
		TSharedPtr<FJsonObject> bufferViewJson = stateBufferView->ToJson();
		serializedBufferViews.Add(MakeShared<FJsonValueObject>(bufferViewJson));
	}
	InG4MFJson->SetArrayField(TEXT("bufferViews"), serializedBufferViews);
	// Serialize accessors (only relevant if buffer views exist).
	const int32 accessorCount = InG4MFState->G4MFAccessors.Num();
	if (accessorCount == 0)
	{
		return true; // No accessors to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> serializedAccessors;
	for (int32 accessorIndex = 0; accessorIndex < accessorCount; accessorIndex++)
	{
		const TObjectPtr<UG4MFAccessor4D>& stateAccessor = InG4MFState->G4MFAccessors[accessorIndex];
		if (!IsValid(stateAccessor))
		{
			return false;
		}
		TSharedPtr<FJsonObject> accessorJson = stateAccessor->ToJson();
		serializedAccessors.Add(MakeShared<FJsonValueObject>(accessorJson));
	}
	InG4MFJson->SetArrayField(TEXT("accessors"), serializedAccessors);
	return true;
}

bool UG4MFDocument4D::ExportSerializeBufferData(UG4MFState4D* InG4MFState, const bool bShouldSeparateBuffersIntoFiles)
{
	if (InG4MFState == nullptr) { return false; }
	const int32 bufferCount = InG4MFState->G4MFBuffers.Num();
	if (bufferCount == 0 || !InG4MFState->G4MFJson->HasField(TEXT("buffers")))
	{
		return true; // No buffers to serialize.
	}
	TArray<TSharedPtr<FJsonValue>> jsonBuffers = InG4MFState->G4MFJson->GetArrayField(TEXT("buffers"));
	if (jsonBuffers.Num() != bufferCount)
	{
		return false;
	}
	const FString filePrefix = FPaths::GetBaseFilename(InG4MFState->G4MFFilename);
	for (int32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		TSharedPtr<FJsonObject> jsonBufferObject = jsonBuffers[bufferIndex]->AsObject();
		if (jsonBufferObject == nullptr)
		{
			return false;
		}
		const TArray64<uint8> encodedBuffer = ExportEncodeChunkData(InG4MFState, InG4MFState->G4MFBuffers[bufferIndex].Data);
		if (bShouldSeparateBuffersIntoFiles)
		{
			const FString bufferRelPath = filePrefix + FString::Printf(TEXT("_buffer%d.bin"), bufferIndex);
			const FString fullFilePath = FPaths::Combine(InG4MFState->G4MFBasePath, bufferRelPath);
			if (FFileHelper::SaveArrayToFile(encodedBuffer, *fullFilePath))
			{
				jsonBufferObject->SetStringField(TEXT("uri"), bufferRelPath);
				continue;
			}
			else
			{
				UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF export: Failed to write buffer data to file: %s. Writing as base64 instead as fallback."), *fullFilePath);
			}
		}
		if (encodedBuffer.Num() > MAX_int32)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Buffer is too large to encode as a base64 URI."));
			return false;
		}
		TArray<uint8> encodedBuffer32;
		encodedBuffer32.Append(encodedBuffer.GetData(), (int32)encodedBuffer.Num());
		const FString base64Buffer = FBase64::Encode(encodedBuffer32);
		jsonBufferObject->SetStringField(TEXT("uri"), FString::Printf(TEXT("data:application/octet-stream;base64,%s"), *base64Buffer));
	}
	return true;
}

FString UG4MFDocument4D::ExportSerializeJsonToString(TSharedPtr<FJsonObject>& InG4MFJson, const bool bInPrettyPrint)
{
	if (InG4MFJson == nullptr) { return FString(); }
	FString outputString;
	if (bInPrettyPrint)
	{
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> prettyWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&outputString);
		FJsonSerializer::Serialize(InG4MFJson, prettyWriter);
	}
	else
	{
		TSharedRef<TJsonWriter<>> compactWriter = TJsonWriterFactory<>::Create(&outputString);
		FJsonSerializer::Serialize(InG4MFJson, compactWriter);
	}
	return outputString;
}

TArray64<uint8> UG4MFDocument4D::ExportEncodeChunkData(UG4MFState4D* InG4MFState, const TArray64<uint8>& InBufferData)
{
	switch (EncodingFormat)
	{
		case EG4MFEncodingFormat4D::Plain:
		{
			return InBufferData;
		}
		case EG4MFEncodingFormat4D::Zstd:
		{
			// TODO: Implement Zstd compression.
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Zstd compression not yet implemented."));
			return TArray64<uint8>();
		}
		case EG4MFEncodingFormat4D::Unknown:
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Unknown compression format."));
			return TArray64<uint8>();
		}
	}
	return TArray64<uint8>();
}

TArray64<uint8> UG4MFDocument4D::ExportWriteToByteArrayInternal(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return TArray64<uint8>(); }
	if (!InG4MFState->G4MFJson.IsValid())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON data has not been serialized."));
		return TArray64<uint8>();
	}
	if (!IsEncodingFormatSupported(EncodingFormat))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Invalid, unknown, or unsupported encoding format. Cannot export G4MF file."));
		return TArray64<uint8>();
	}
	if (!InG4MFState->G4MFJson->HasField(TEXT("asset")))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON data is missing 'asset' field."));
		return TArray64<uint8>();
	}
	// Add binary buffer chunks to the file size.
	TArray<bool> buffersShouldWriteToChunk;
	int64 bufferChunkIndex = 1;
	// "Automatic" is always embedded in this case anyway, so pass `-1` for `p_blob_size` which returns false (flipped to true here).
	const bool shouldEmbedBuffers = !InG4MFState->ShouldSeparateBinaryBlobs(-1);
	if (shouldEmbedBuffers && !InG4MFState->G4MFBuffers.IsEmpty())
	{
		if (!InG4MFState->G4MFJson->HasField(TEXT("buffers")))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON data is missing 'buffers' field while state has buffers to embed."));
			return TArray64<uint8>();
		}
		TArray<TSharedPtr<FJsonValue>> jsonBuffers = InG4MFState->G4MFJson->GetArrayField(TEXT("buffers"));
		const int32 bufferCount = InG4MFState->G4MFBuffers.Num();
		if (jsonBuffers.Num() != bufferCount)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON 'buffers' array size does not match the number of buffers in state."));
			return TArray64<uint8>();
		}
		buffersShouldWriteToChunk.SetNum(bufferCount);
		for (int32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
		{
			TSharedPtr<FJsonObject> jsonBufferObject = jsonBuffers[bufferIndex]->AsObject();
			if (jsonBufferObject == nullptr)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON 'buffers' array contains a non-object entry."));
				return TArray64<uint8>();
			}
			if (jsonBufferObject->HasField(TEXT("uri")))
			{
				continue; // This buffer is already referenced by a URI, so skip it.
			}
			jsonBufferObject->SetNumberField(TEXT("chunk"), bufferChunkIndex);
			bufferChunkIndex++;
			buffersShouldWriteToChunk[bufferIndex] = true;
			jsonBuffers[bufferIndex] = MakeShared<FJsonValueObject>(jsonBufferObject);
		}
		InG4MFState->G4MFJson->SetArrayField(TEXT("buffers"), jsonBuffers);
	}
	// Encode the JSON chunk and all buffer chunks, and calculate the total file size.
	const FString jsonString = ExportSerializeJsonToString(InG4MFState->G4MFJson, false);
	// Minimum possible valid JSON is `{"asset":{"dimension":4}}` (25 bytes).
	if (jsonString.Len() < 25)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize JSON data to string."));
		return TArray64<uint8>();
	}
	const FTCHARToUTF8 jsonUtf8(*jsonString);
	const TArray64<uint8> jsonBytes(reinterpret_cast<const uint8*>(jsonUtf8.Get()), jsonUtf8.Length());
	const TArray64<uint8> jsonEncoded = ExportEncodeChunkData(InG4MFState, jsonBytes);
	const uint64 jsonEncodedSize = jsonEncoded.Num();
	constexpr uint64 CHUNK_HEADER_SIZE = 16;
	constexpr uint64 FILE_HEADER_SIZE = 16;
	if (jsonEncodedSize > (uint64)MAX_int64 - FILE_HEADER_SIZE - CHUNK_HEADER_SIZE)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: JSON chunk is too large to fit in an Unreal array."));
		return TArray64<uint8>();
	}
	uint64 totalFileSize = FILE_HEADER_SIZE + CHUNK_HEADER_SIZE + jsonEncodedSize;
	TArray<FG4MFBuffer4D> buffersEncoded;
	if (shouldEmbedBuffers && !InG4MFState->G4MFBuffers.IsEmpty())
	{
		// We can skip several of the checks because they are handled in the previous block.
		const int32 bufferCount = InG4MFState->G4MFBuffers.Num();
		buffersEncoded.SetNum(bufferCount);
		for (int32 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
		{
			if (!buffersShouldWriteToChunk[bufferIndex])
			{
				continue;
			}
			// The start of each chunk needs to be padded to 16 bytes, so pad the previous chunk to 16 bytes if needed.
			if (totalFileSize > (uint64)MAX_int64 - CHUNK_ALIGNMENT_BITMASK)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Binary file size overflow while aligning a buffer chunk."));
				return TArray64<uint8>();
			}
			totalFileSize = FMath::DivideAndRoundUp(totalFileSize, (uint64)16) * (uint64)16;
			TArray64<uint8> bufferEncoded = ExportEncodeChunkData(InG4MFState, InG4MFState->G4MFBuffers[bufferIndex].Data);
			if (totalFileSize > (uint64)MAX_int64 - CHUNK_HEADER_SIZE ||
				(uint64)bufferEncoded.Num() > (uint64)MAX_int64 - totalFileSize - CHUNK_HEADER_SIZE)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Binary file is too large to fit in an Unreal array."));
				return TArray64<uint8>();
			}
			totalFileSize += CHUNK_HEADER_SIZE + bufferEncoded.Num();
			buffersEncoded[bufferIndex].Data = MoveTemp(bufferEncoded);
		}
	}
	// Done gathering information, now create the byte array.
	TArray64<uint8> fileBytes;
	fileBytes.SetNumUninitialized(totalFileSize);
	// Write the file header.
	uint8* fileBytesPtrw = fileBytes.GetData();
	*(uint32*)fileBytesPtrw = (uint32)0x464D3447; // ASCII string "G4MF" in little-endian.
	uint64 writeOffset = 4;
	*(uint32*)(fileBytesPtrw + writeOffset) = (uint32)0x00000000; // Version 0
	writeOffset += 4;
	*(uint64*)(fileBytesPtrw + writeOffset) = totalFileSize; // File size
	writeOffset += 8;
	// Write the JSON chunk.
	*(uint32*)(fileBytesPtrw + writeOffset) = (uint32)0x4E4F534A; // ASCII string "JSON" in little-endian.
	writeOffset += 4;
	*(uint32*)(fileBytesPtrw + writeOffset) = (uint32)EncodingFormat;
	writeOffset += 4;
	*(uint64*)(fileBytesPtrw + writeOffset) = jsonEncodedSize; // JSON size after encoding.
	writeOffset += 8;
	FMemory::Memcpy(fileBytesPtrw + writeOffset, jsonEncoded.GetData(), jsonEncodedSize);
	writeOffset += jsonEncodedSize;
	// Write buffer chunks.
	if (shouldEmbedBuffers)
	{
		bool paddedJson = false;
		for (int32 bufferIndex = 0; bufferIndex < buffersEncoded.Num(); bufferIndex++)
		{
			if (!buffersShouldWriteToChunk[bufferIndex])
			{
				continue;
			}
			// Pad the previous chunk to 16 bytes if needed.
			const uint64 writeOffsetPadded = FMath::DivideAndRoundUp(writeOffset, (uint64)16) * (uint64)16;
			for (uint64 padIndex = writeOffset; padIndex < writeOffsetPadded; padIndex++)
			{
				// Pad the previous chunk with zero bytes, or spaces after a plainly encoded JSON chunk.
				const bool useSpaces = (EncodingFormat == EG4MFEncodingFormat4D::Plain && !paddedJson);
				*(fileBytesPtrw + writeOffset) = useSpaces ? (uint8)0x20 : (uint8)0x00;
				writeOffset++;
			}
			paddedJson = true;
			const TArray64<uint8>& bufferEncoded = buffersEncoded[bufferIndex].Data;
			const uint64 bufferEncodedSize = bufferEncoded.Num();
			*(uint32*)(fileBytesPtrw + writeOffset) = (uint32)0x424F4C42; // ASCII string "BLOB" in little-endian.
			writeOffset += 4;
			*(uint32*)(fileBytesPtrw + writeOffset) = (uint32)EncodingFormat;
			writeOffset += 4;
			*(uint64*)(fileBytesPtrw + writeOffset) = bufferEncodedSize; // Buffer size after encoding.
			writeOffset += 8;
			if (bufferEncodedSize > 0)
			{
				FMemory::Memcpy(fileBytesPtrw + writeOffset, bufferEncoded.GetData(), bufferEncodedSize);
			}
			writeOffset += bufferEncodedSize;
		}
	}
	// Final sanity check.
	if (writeOffset != totalFileSize)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Final write offset does not match calculated total file size."));
		return TArray64<uint8>();
	}
	return fileBytes;
}

// Import process.

bool UG4MFDocument4D::ImportReadFromBinaryFile(UG4MFState4D* InG4MFState, const TUniquePtr<IFileHandle>& InFileHandle)
{
	if (InG4MFState == nullptr || !InFileHandle.IsValid()) { return false; }
	// Allow starting a file read in the middle, in case callers want to read a sub-file from a larger file.
	const uint64 fileReadOffset = InFileHandle->Tell();
	const uint64 fileTotalLength = InFileHandle->Size();
	if (fileReadOffset > fileTotalLength)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File read offset is beyond the end of the file. File is corrupted."));
		return false;
	}
	const uint64 fileRemainingSize = fileTotalLength - fileReadOffset;
	if (fileRemainingSize < 32)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File is too small to be a valid G4MF file."));
		return false;
	}
	// Read the file header and validate it.
	if (FileReadUint32(InFileHandle) != 0x464D3447) // ASCII string "G4MF" in little-endian.
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File is not a valid G4MF file."));
		return false;
	}
	if (FileReadUint32(InFileHandle) > 0x00000000)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: G4MF file version is not supported. Unreal 4D cannot import this G4MF file."));
		return false;
	}
	const uint64 declaredFileSize = FileReadUint64(InFileHandle);
	if (declaredFileSize < 32)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File is too small to be a valid G4MF file. File is corrupted."));
		return false;
	}
	if (declaredFileSize > (uint64)INT64_MAX)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File is too large to be a valid G4MF file. File is corrupted."));
		return false;
	}
	if (declaredFileSize > fileRemainingSize)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared file size exceeds available file size. File is corrupted."));
		return false;
	}
	// Read the chunk headers to find where each chunk starts.
	// The first JSON chunk is the G4MF JSON chunk; it does not need to be the first chunk in the file.
	TArray<EG4MFEncodingFormat4D> chunkEncodings;
	TArray64<int64> chunkDataSizes;
	TArray64<int64> chunkDataStarts;
	int64 jsonChunkIndex = -1;
	uint64 chunkStartsReadOffset = InFileHandle->Tell();
	const uint64 declaredFileEnd = fileReadOffset + declaredFileSize;
	while (chunkStartsReadOffset + CHUNK_ALIGNMENT_BITMASK < declaredFileEnd)
	{
		if (!InFileHandle->Seek(chunkStartsReadOffset))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to seek to a chunk header."));
			return false;
		}
		const uint32 chunkType = FileReadUint32(InFileHandle);
		if (jsonChunkIndex == -1 && chunkType == 0x4E4F534A) // ASCII string "JSON" in little-endian.
		{
			jsonChunkIndex = chunkDataStarts.Num();
		}
		const uint32 chunkEncodingUint32 = FileReadUint32(InFileHandle);
		const EG4MFEncodingFormat4D chunkEncodingFormat = ValidateEncodingFormat(chunkEncodingUint32);
		chunkEncodings.Add(chunkEncodingFormat);
		const uint64 chunkSize = FileReadUint64(InFileHandle);
		if (chunkSize > INT64_MAX) // Not UINT64_MAX.
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Chunk size exceeds maximum supported size. File is corrupted."));
			return false;
		}
		const uint64 paddedChunkSize = (chunkSize + CHUNK_ALIGNMENT_BITMASK) & ~CHUNK_ALIGNMENT_BITMASK;
		if (chunkStartsReadOffset > UINT64_MAX - paddedChunkSize)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Integer overflow when calculating next chunk start offset. File is corrupted."));
			return false;
		}
		chunkDataSizes.Add((int64)chunkSize);
		chunkStartsReadOffset += 16;
		chunkDataStarts.Add((int64)chunkStartsReadOffset);
		if (chunkSize > declaredFileEnd - chunkStartsReadOffset)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared chunk size goes beyond the file size. File is corrupted."));
			return false;
		}
		chunkStartsReadOffset += paddedChunkSize;
	}
	if (jsonChunkIndex == -1)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: No JSON chunk found in G4MF file. File is corrupted."));
		return false;
	}
	// Read and parse the JSON chunk.
	TSharedPtr<FJsonObject> g4mfJson;
	TArray64<int64> bufferChunkIndices;
	TArray64<int64> bufferChunkDeclaredDecodedByteLengths;
	{
		if (!InFileHandle->Seek(chunkDataStarts[jsonChunkIndex]))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to seek to the JSON chunk."));
			return false;
		}
		TArray64<uint8> jsonChunkEncoded;
		const int64 jsonChunkDataSize = chunkDataSizes[jsonChunkIndex];
		jsonChunkEncoded.SetNumUninitialized(jsonChunkDataSize);
		if (jsonChunkDataSize > 0 && !InFileHandle->Read(jsonChunkEncoded.GetData(), jsonChunkDataSize))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to read the JSON chunk."));
			return false;
		}
		const EG4MFEncodingFormat4D jsonChunkEncodingFormat = chunkEncodings[jsonChunkIndex];
		if (!IsEncodingFormatSupported(jsonChunkEncodingFormat))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Support for reading encoding format %d is not implemented. Unreal 4D cannot import this G4MF file."), (int32)jsonChunkEncodingFormat);
			return false;
		}
		const TArray64<uint8> jsonChunk = ImportDecodeChunkData(jsonChunkEncoded, (int64)0, jsonChunkDataSize, jsonChunkEncodingFormat);
		// Parse the JSON data.
		const FUTF8ToTCHAR jsonText(reinterpret_cast<const ANSICHAR*>(jsonChunk.GetData()), jsonChunk.Num());
		const FString jsonString(jsonText.Length(), jsonText.Get());
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
		if (!FJsonSerializer::Deserialize(reader, g4mfJson) || !g4mfJson.IsValid())
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse JSON chunk."));
			return false;
		}
		InG4MFState->G4MFJson = g4mfJson;
		// Parse the JSON structure of only the buffers, so we can know which chunks to read and decode.
		if (!ImportParseBuffers(InG4MFState, g4mfJson.Get(), &bufferChunkIndices, &bufferChunkDeclaredDecodedByteLengths))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse G4MF buffers when reading from binary G4MF file."));
			return false;
		}
	}
	// Read and decode any chunk data needed by a buffer.
	for (int64 bufferIndex = 0; bufferIndex < bufferChunkIndices.Num(); bufferIndex++)
	{
		const int64 chunkIndex = bufferChunkIndices[bufferIndex];
		if (chunkIndex == -1)
		{
			continue;
		}
		if (!chunkDataStarts.IsValidIndex(chunkIndex))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer chunk index %lld is out of range for binary file with %lld chunks. File is corrupted."), chunkIndex, chunkDataStarts.Num());
			return false;
		}
		if (!InFileHandle->Seek(chunkDataStarts[chunkIndex]))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to seek to buffer chunk %lld."), chunkIndex);
			return false;
		}
		const int64 chunkDataSize = chunkDataSizes[chunkIndex];
		TArray64<uint8> chunkDataEncoded;
		chunkDataEncoded.SetNumUninitialized(chunkDataSize);
		if (chunkDataSize > 0 && !InFileHandle->Read(chunkDataEncoded.GetData(), chunkDataSize))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to read buffer chunk %lld."), chunkIndex);
			return false;
		}
		const EG4MFEncodingFormat4D chunkEncodingFormat = chunkEncodings[chunkIndex];
		if (!IsEncodingFormatSupported(chunkEncodingFormat))
		{
			const FString friendly = Uint32ToAsciiString((uint32)chunkEncodingFormat, false);
			const FString numberHex = FString::Printf(TEXT("%08X"), (uint32)chunkEncodingFormat);
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Support for reading \"%s\" (0x%s) encoded data is not implemented. Unreal 4D cannot import this G4MF file."), *friendly, *numberHex);
			return false;
		}
		TArray64<uint8> chunkDataDecoded = ImportDecodeChunkData(chunkDataEncoded, (int64)0, chunkDataSize, chunkEncodingFormat);
		// The data has been read in now, so check that the size is at least the declared size, and truncate if larger.
		const int64 declaredDecodedByteLength = bufferChunkDeclaredDecodedByteLengths[bufferIndex];
		if (chunkDataDecoded.Num() < declaredDecodedByteLength)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer size is not at least the declared size. File is corrupted."));
			return false;
		}
		chunkDataDecoded.SetNum(declaredDecodedByteLength);
		InG4MFState->G4MFBuffers[bufferIndex] = FG4MFBuffer4D(chunkDataDecoded);
	}
	// Parse the rest of the G4MF JSON data.
	if (!ImportParseJsonData(InG4MFState, g4mfJson.Get()))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse G4MF JSON data when reading from binary G4MF file."));
		return false;
	}
	return true;
}

TArray64<uint8> UG4MFDocument4D::ImportDecodeChunkData(const TArray64<uint8>& InFileOrChunkData, const int64 InChunkDataOffset, const int64 InChunkDataRawSize, const EG4MFEncodingFormat4D InChunkEncodingFormat)
{
	const int64 sourceDataSize = InFileOrChunkData.Num();
	if (InChunkDataOffset < 0 || InChunkDataRawSize < 0 ||
		InChunkDataOffset > sourceDataSize ||
		InChunkDataRawSize > sourceDataSize - InChunkDataOffset)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Chunk data range is outside the source byte array."));
		return TArray64<uint8>();
	}
	switch (InChunkEncodingFormat)
	{
		case EG4MFEncodingFormat4D::Plain:
		{
			if (InChunkDataOffset == 0 && InChunkDataRawSize == InFileOrChunkData.Num())
			{
				return InFileOrChunkData;
			}
			return TArray64<uint8>(InFileOrChunkData.GetData() + InChunkDataOffset, InChunkDataRawSize);
		} break;
		case EG4MFEncodingFormat4D::Zstd:
		{
			// TODO: Implement Zstd decompression.
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF import: Zstd decompression not yet implemented."));
			return TArray64<uint8>();
		} break;
		default:
			break;
	}
	const FString friendly = Uint32ToAsciiString((uint32)InChunkEncodingFormat, false);
	const FString numberHex = FString::Printf(TEXT("%08X"), (uint32)InChunkEncodingFormat);
	UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Support for reading \"%s\" (0x%s) encoded data is not implemented."), *friendly, *numberHex);
	return TArray64<uint8>();
}

// The return parameters are used when parsing binary G4MF files.
// The callers use different APIs (IFileHandle vs TArray64<uint8>) so we need to keep the output of this function generic.
bool UG4MFDocument4D::ImportParseBuffers(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson, TArray64<int64>* OutBufferChunkIndices, TArray64<int64>* OutBufferChunkDeclaredDecodedByteLengths)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	ensure((OutBufferChunkIndices == nullptr) == (OutBufferChunkDeclaredDecodedByteLengths == nullptr));
	InG4MFState->ExternalDataMode = EG4MFExternalDataMode4D::EmbedEverything;
	if (!InG4MFJson->HasField(TEXT("buffers")))
	{
		return true; // No buffers to parse.
	}
	const TArray<TSharedPtr<FJsonValue>> jsonBuffers = InG4MFJson->GetArrayField(TEXT("buffers"));
	const int64 bufferCount = jsonBuffers.Num();
	if (bufferCount == 0)
	{
		return true; // No buffers to parse.
	}
	InG4MFState->G4MFBuffers.SetNum(bufferCount);
	if (OutBufferChunkIndices != nullptr)
	{
		OutBufferChunkIndices->SetNumUninitialized(bufferCount);
		OutBufferChunkDeclaredDecodedByteLengths->SetNumUninitialized(bufferCount);
		for (int64 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
		{
			(*OutBufferChunkIndices)[bufferIndex] = -1;
			(*OutBufferChunkDeclaredDecodedByteLengths)[bufferIndex] = -1;
		}
	}
	for (int64 bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++)
	{
		const TSharedPtr<FJsonObject> jsonBuffer = jsonBuffers[bufferIndex]->AsObject();
		if (!jsonBuffer.IsValid() || !jsonBuffer->HasField(TEXT("byteLength")))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer is missing required field 'byteLength'. Aborting file import."));
			return false;
		}
		// If the buffer is encoded, this refers to the byte length after decoding.
		const int64 declaredDecodedByteLength = jsonBuffer->GetIntegerField(TEXT("byteLength"));
		if (declaredDecodedByteLength < 0)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer 'byteLength' is negative. Aborting file import."));
			return false;
		}
		if (OutBufferChunkDeclaredDecodedByteLengths != nullptr)
		{
			(*OutBufferChunkDeclaredDecodedByteLengths)[bufferIndex] = declaredDecodedByteLength;
		}
		EG4MFEncodingFormat4D encodingFormat = EG4MFEncodingFormat4D::Plain;
		if (jsonBuffer->HasField(TEXT("encoding")))
		{
			const FString encodingStr = jsonBuffer->GetStringField(TEXT("encoding"));
			encodingFormat = ValidateEncodingFormat(AsciiStringToUint32(encodingStr));
		}
		FG4MFBuffer4D stateBuffer = InG4MFState->G4MFBuffers[bufferIndex];
		if (jsonBuffer->HasField(TEXT("uri")))
		{
			if (!IsEncodingFormatSupported(encodingFormat))
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer URI uses an unsupported encoding format. Aborting file import."));
				return false;
			}
			const FString uri = jsonBuffer->GetStringField(TEXT("uri"));
			if (uri.StartsWith(TEXT("data:")))
			{
				TArray<FString> split;
				uri.ParseIntoArray(split, TEXT(";base64,"), false);
				if (split.Num() != 2)
				{
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer URI is malformed. Expected 'data:application/octet-stream;base64,<base64 data>'. Aborting file import."));
					return false;
				}
				TArray<uint8> rawBufferData32;
				bool decodedSuccessfully = FBase64::Decode(split[1], rawBufferData32, EBase64Mode::Standard);
				if (!decodedSuccessfully)
				{
					decodedSuccessfully = FBase64::Decode(split[1], rawBufferData32, EBase64Mode::UrlSafe);
					if (decodedSuccessfully)
					{
						// Shouldn't happen, but accept it for robustness.
						UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF import: Buffer URI base64 data is not in the standard format."));
					}
				}
				if (!decodedSuccessfully)
				{
					UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to decode base64 buffer data. Aborting file import."));
					return false;
				}
				TArray64<uint8> rawBufferData;
				rawBufferData.Append(rawBufferData32.GetData(), rawBufferData32.Num());
				stateBuffer.Data = ImportDecodeChunkData(rawBufferData, (int64)0, rawBufferData.Num(), encodingFormat);
			}
			else
			{
				// Infer the external data mode on import in case the user wishes
				// to round-trip the G4MF file back out of Unreal later.
				InG4MFState->ExternalDataMode = EG4MFExternalDataMode4D::SeparateBinaryBlobs;
				const FString bufferPath = FPaths::Combine(InG4MFState->G4MFBasePath, uri);
				TArray64<uint8> fileRawData;
				if (FFileHelper::LoadFileToArray(fileRawData, *bufferPath))
				{
					stateBuffer.Data = ImportDecodeChunkData(fileRawData, (int64)0, fileRawData.Num(), encodingFormat);
				}
				else
				{
					// The file is not valid, but only fail if the buffer is empty. It may have been filled by a chunk.
					if (stateBuffer.Data.IsEmpty())
					{
						UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to open buffer file: %s. Aborting file import."), *bufferPath);
						return false;
					}
				}
			}
			// byteLength is the decoded length. Permit padding beyond it, but reject
			// truncated data and discard any padding after decoding.
			if (stateBuffer.Data.Num() < declaredDecodedByteLength)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer size is smaller than its declared byteLength. Aborting file import."));
				return false;
			}
			stateBuffer.Data.SetNum(declaredDecodedByteLength);
		}
		else if (jsonBuffer->HasField(TEXT("chunk")))
		{
			if (OutBufferChunkIndices != nullptr)
			{
				(*OutBufferChunkIndices)[bufferIndex] = jsonBuffer->GetIntegerField(TEXT("chunk"));
			}
			else
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer contains 'chunk' field in a text-based G4MF file. File is invalid. Aborting file import."));
				return false;
			}
		}
		else
		{
			// Fallback behavior: If the buffer is missing both 'uri' and 'chunk', use implicit indices for chunks.
			if (OutBufferChunkIndices != nullptr)
			{
				// Don't log a warning here, because it's fine if Unreal 4D encounters an older G4MF file that lacks the 'chunk' field.
				// Legacy fallback only: Older files placed the JSON at chunk 0, so buffer 0 -> chunk 1, buffer 1 -> chunk 2, etc.
				(*OutBufferChunkIndices)[bufferIndex] = bufferIndex + 1;
			}
			else
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer is missing 'uri' field required for a text-based G4MF file. File is invalid. Aborting file import."));
				return false;
			}
		}
		InG4MFState->G4MFBuffers[bufferIndex] = stateBuffer;
	}
	return true;
}

// Note: This function excludes parsing buffers, because those need to be done in sync with
// file loading, and therefore are handled separately in `ImportParseBuffers()` above.
bool UG4MFDocument4D::ImportParseJsonData(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!ImportParseAssetHeader(InG4MFState, InG4MFJson))
	{
		return false;
	}
	// Parse node names first to ensure that their names take priority over all other named objects.
	if (!ImportParseNodeNames(InG4MFState, InG4MFJson))
	{
		return false;
	}
	// Parse everything else in dependency order.
	if (!ImportParseBufferViews(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseAccessors(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseFiles(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseTextures(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseMaterials(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseMeshes(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseShapes(InG4MFState, InG4MFJson))
	{
		return false;
	}
	if (!ImportParseNodes(InG4MFState, InG4MFJson))
	{
		return false;
	}
	return true;
}

bool UG4MFDocument4D::ImportParseAssetHeader(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("asset")))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Missing required 'asset' header."));
		return false;
	}
	const TSharedPtr<FJsonObject> assetHeader = InG4MFJson->GetObjectField(TEXT("asset"));
	if (!assetHeader.IsValid() || !assetHeader->HasField(TEXT("dimension")))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Missing required asset header field 'dimension'."));
		return false;
	}
	InG4MFState->DeclaredDimension = assetHeader->GetIntegerField(TEXT("dimension"));
	if (InG4MFState->DeclaredDimension != 4)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF import: Trying to import a non-4D G4MF as 4D. This may not work correctly."));
	}
	return true;
}

bool UG4MFDocument4D::ImportParseBufferViews(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("bufferViews")))
	{
		return true; // No buffer views to parse.
	}
	TArray<TSharedPtr<FJsonValue>> jsonBufferViews = InG4MFJson->GetArrayField(TEXT("bufferViews"));
	const int32 bufferViewCount = jsonBufferViews.Num();
	if (bufferViewCount == 0)
	{
		return true; // No buffer views to parse.
	}
	InG4MFState->G4MFBufferViews.SetNum(bufferViewCount);
	for (int32 bufferViewIndex = 0; bufferViewIndex < bufferViewCount; bufferViewIndex++)
	{
		TSharedPtr<FJsonObject> jsonBufferViewObject = jsonBufferViews[bufferViewIndex]->AsObject();
		if (!jsonBufferViewObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFBufferViews[bufferViewIndex] = UG4MFBufferView4D::FromJson(InG4MFState, jsonBufferViewObject.Get());
	}
	return true;
}

bool UG4MFDocument4D::ImportParseAccessors(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("accessors")))
	{
		return true; // No accessors to parse.
	}
	TArray<TSharedPtr<FJsonValue>> jsonAccessors = InG4MFJson->GetArrayField(TEXT("accessors"));
	const int32 accessorCount = jsonAccessors.Num();
	if (accessorCount == 0)
	{
		return true; // No accessors to parse.
	}
	InG4MFState->G4MFAccessors.SetNum(accessorCount);
	for (int32 accessorIndex = 0; accessorIndex < accessorCount; accessorIndex++)
	{
		TSharedPtr<FJsonObject> jsonAccessorObject = jsonAccessors[accessorIndex]->AsObject();
		if (!jsonAccessorObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFAccessors[accessorIndex] = UG4MFAccessor4D::FromJson(InG4MFState, jsonAccessorObject.Get());
	}
	return true;
}

bool UG4MFDocument4D::ImportParseFiles(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("files")))
	{
		return true; // No files is valid.
	}
	TArray<TSharedPtr<FJsonValue>> jsonFiles = InG4MFJson->GetArrayField(TEXT("files"));
	const int32 fileCount = jsonFiles.Num();
	if (fileCount == 0)
	{
		return true; // No files is valid.
	}
	InG4MFState->G4MFFiles.SetNum(fileCount);
	for (int32 fileIndex = 0; fileIndex < fileCount; fileIndex++)
	{
		TSharedPtr<FJsonObject> jsonFileObject = jsonFiles[fileIndex]->AsObject();
		if (!jsonFileObject.IsValid())
		{
			return false;
		}
		FString mimeType = TEXT("");
		if (jsonFileObject->HasField(TEXT("mimeType")))
		{
			mimeType = jsonFileObject->GetStringField(TEXT("mimeType"));
		}
		// The type of generated class depends on what type of file this is.
		if (mimeType.StartsWith(TEXT("model/")))
		{
			UG4MFModel4D* model = UG4MFModel4D::FromJson(InG4MFState, jsonFileObject.Get());
			model->ModelG4MFDocument = this;
			InG4MFState->G4MFFiles[fileIndex] = model;
		}
		else
		{
			InG4MFState->G4MFFiles[fileIndex] = UG4MFFileReference4D::FileReferenceFromJson(InG4MFState, jsonFileObject.Get());
		}
		const bool success = InG4MFState->G4MFFiles[fileIndex]->ImportParseFileData(InG4MFState);
		if (!success)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to preload file data. Aborting file import."));
			return false;
		}
	}
	// Reserve unique names for files.
	for (int32 fileIndex = 0; fileIndex < InG4MFState->G4MFFiles.Num(); fileIndex++)
	{
		UG4MFFileReference4D* file = InG4MFState->G4MFFiles[fileIndex];
		if (file == nullptr) { return false; }
		FString fileName = file->GetItemName();
		if (fileName.IsEmpty())
		{
			fileName = FString::Printf(TEXT("File%d"), fileIndex);
			file->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(fileName));
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseTextures(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("textures")))
	{
		return true; // No textures is valid.
	}
	TArray<TSharedPtr<FJsonValue>> jsonTextures = InG4MFJson->GetArrayField(TEXT("textures"));
	const int32 textureCount = jsonTextures.Num();
	if (textureCount == 0)
	{
		return true; // No textures is valid.
	}
	InG4MFState->G4MFTextures.SetNum(textureCount);
	for (int32 textureIndex = 0; textureIndex < textureCount; textureIndex++)
	{
		TSharedPtr<FJsonObject> jsonTextureObject = jsonTextures[textureIndex]->AsObject();
		if (!jsonTextureObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFTextures[textureIndex] = UG4MFTexture4D::FromJson(InG4MFState, jsonTextureObject.Get());
	}
	// Reserve unique names for textures.
	for (int32 textureIndex = 0; textureIndex < InG4MFState->G4MFTextures.Num(); textureIndex++)
	{
		UG4MFTexture4D* texture = InG4MFState->G4MFTextures[textureIndex];
		if (texture == nullptr) { return false; }
		FString textureName = texture->GetItemName();
		if (textureName.IsEmpty())
		{
			textureName = FString::Printf(TEXT("Texture%d"), textureIndex);
			texture->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(textureName));
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseMaterials(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("materials")))
	{
		return true; // No materials is valid.
	}
	TArray<TSharedPtr<FJsonValue>> jsonMaterials = InG4MFJson->GetArrayField(TEXT("materials"));
	const int32 materialCount = jsonMaterials.Num();
	if (materialCount == 0)
	{
		return true; // No materials is valid.
	}
	InG4MFState->G4MFMaterials.SetNum(materialCount);
	for (int32 materialIndex = 0; materialIndex < materialCount; materialIndex++)
	{
		TSharedPtr<FJsonObject> jsonMaterialObject = jsonMaterials[materialIndex]->AsObject();
		if (!jsonMaterialObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFMaterials[materialIndex] = UG4MFMaterial4D::FromJson(InG4MFState, jsonMaterialObject.Get());
	}
	// Reserve unique names for materials.
	for (int32 materialIndex = 0; materialIndex < InG4MFState->G4MFMaterials.Num(); materialIndex++)
	{
		UG4MFMaterial4D* material = InG4MFState->G4MFMaterials[materialIndex];
		if (material == nullptr) { return false; }
		FString materialName = material->GetItemName();
		if (materialName.IsEmpty())
		{
			materialName = FString::Printf(TEXT("Material%d"), materialIndex);
			material->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(materialName));
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseMeshes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("meshes")))
	{
		return true; // No meshes is valid.
	}
	TArray<TSharedPtr<FJsonValue>> jsonMeshes = InG4MFJson->GetArrayField(TEXT("meshes"));
	const int32 meshCount = jsonMeshes.Num();
	if (meshCount == 0)
	{
		return true; // No meshes is valid.
	}
	InG4MFState->G4MFMeshes.SetNum(meshCount);
	for (int32 meshIndex = 0; meshIndex < meshCount; meshIndex++)
	{
		TSharedPtr<FJsonObject> jsonMeshObject = jsonMeshes[meshIndex]->AsObject();
		if (!jsonMeshObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFMeshes[meshIndex] = UG4MFMesh4D::FromJson(InG4MFState, jsonMeshObject.Get());
	}
	// Reserve unique names for meshes and mesh surfaces.
	for (int32 meshIndex = 0; meshIndex < InG4MFState->G4MFMeshes.Num(); meshIndex++)
	{
		UG4MFMesh4D* mesh = InG4MFState->G4MFMeshes[meshIndex];
		if (mesh == nullptr) { return false; }
		FString meshName = mesh->GetItemName();
		if (meshName.IsEmpty())
		{
			meshName = FString::Printf(TEXT("Mesh%d"), meshIndex);
			mesh->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(meshName));
		}
		for (int32 surfaceIndex = 0; surfaceIndex < mesh->Surfaces.Num(); surfaceIndex++)
		{
			UG4MFMeshSurface4D* surface = mesh->Surfaces[surfaceIndex];
			if (surface == nullptr) { return false; }
			FString surfaceName = surface->GetItemName();
			if (surfaceName.IsEmpty())
			{
				surfaceName = FString::Printf(TEXT("%s_Surface%d"), *meshName, surfaceIndex);
				surface->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(surfaceName));
			}
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseShapes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("shapes")))
	{
		return true; // No shapes is valid.
	}
	TArray<TSharedPtr<FJsonValue>> jsonShapes = InG4MFJson->GetArrayField(TEXT("shapes"));
	const int32 shapeCount = jsonShapes.Num();
	if (shapeCount == 0)
	{
		return true; // No shapes is valid.
	}
	InG4MFState->G4MFShapes.SetNum(shapeCount);
	for (int32 shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++)
	{
		TSharedPtr<FJsonObject> jsonShapeObject = jsonShapes[shapeIndex]->AsObject();
		if (!jsonShapeObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFShapes[shapeIndex] = UG4MFShape4D::FromJson(InG4MFState, jsonShapeObject.Get());
	}
	// Reserve unique names for shapes.
	for (int32 shapeIndex = 0; shapeIndex < InG4MFState->G4MFShapes.Num(); shapeIndex++)
	{
		UG4MFShape4D* shape = InG4MFState->G4MFShapes[shapeIndex];
		if (shape == nullptr) { return false; }
		FString shapeName = shape->GetItemName();
		if (shapeName.IsEmpty())
		{
			shapeName = FString::Printf(TEXT("Shape%d"), shapeIndex);
			shape->SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(shapeName));
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseNodeNames(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("nodes")))
	{
		return true; // No nodes is valid (just not a scene).
	}
	TArray<TSharedPtr<FJsonValue>> jsonNodes = InG4MFJson->GetArrayField(TEXT("nodes"));
	const int32 nodeCount = jsonNodes.Num();
	if (nodeCount == 0)
	{
		return true; // No nodes is valid (just not a scene).
	}
	// Initial name-only parsing of nodes.
	InG4MFState->G4MFNodes.SetNum(nodeCount);
	for (int32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
	{
		TSharedPtr<FJsonObject> jsonNodeObject = jsonNodes[nodeIndex]->AsObject();
		if (!jsonNodeObject.IsValid())
		{
			return false;
		}
		InG4MFState->G4MFNodes[nodeIndex] = UG4MFNode4D::FromJsonNameOnly(InG4MFState, jsonNodeObject.Get());
		if (InG4MFState->G4MFNodes[nodeIndex] == nullptr)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse node at index %d. Aborting file import."), nodeIndex);
			return false;
		}
	}
	return true;
}

bool UG4MFDocument4D::ImportParseNodes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson)
{
	if (InG4MFState == nullptr || InG4MFJson == nullptr) { return false; }
	if (!InG4MFJson->HasField(TEXT("nodes")))
	{
		return true; // No nodes is valid (just not a scene).
	}
	TArray<TSharedPtr<FJsonValue>> jsonNodes = InG4MFJson->GetArrayField(TEXT("nodes"));
	const int32 nodeCount = jsonNodes.Num();
	if (nodeCount == 0)
	{
		return true; // No nodes is valid (just not a scene).
	}
	// Second pass for parsing node content.
	ensure(InG4MFState->G4MFNodes.Num() == nodeCount);
	for (int32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
	{
		TSharedPtr<FJsonObject> jsonNodeObject = jsonNodes[nodeIndex]->AsObject();
		if (!jsonNodeObject.IsValid())
		{
			return false;
		}
		UG4MFNode4D* node = InG4MFState->G4MFNodes[nodeIndex];
		node->ReadNodeEntriesFromJson(InG4MFState, jsonNodeObject.Get());
	}
	// Assign parent index.
	// TODO: Validate that the node graph has no cycles and that each non-root node
	// has at most one parent before recursively generating a component hierarchy.
	// G4MF forbids those graphs, but rejecting malformed input here would avoid
	// unbounded recursion later.
	for (int32 parentIndex = 0; parentIndex < nodeCount; parentIndex++)
	{
		const UG4MFNode4D* node = InG4MFState->G4MFNodes[parentIndex];
		const TArray<int32> childrenIndices = node->ChildrenIndices;
		for (int32 childIndex : childrenIndices)
		{
			if (!InG4MFState->G4MFNodes.IsValidIndex(childIndex))
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Node has invalid child index. Aborting file import."));
				return false;
			}
			InG4MFState->G4MFNodes[childIndex]->ParentIndex = parentIndex;
		}
	}
	// Reserve unique names for nodes and their functional items.
	for (int32 nodeIndex = 0; nodeIndex < InG4MFState->G4MFNodes.Num(); nodeIndex++)
	{
		UG4MFNode4D* node = InG4MFState->G4MFNodes[nodeIndex];
		if (node == nullptr) { return false; }
		InG4MFState->ReserveUniqueNamesForNodeAndSubItems(node, nodeIndex);
	}
	// TODO: Extensions.
	// The root node MUST NOT have a transform as required by the G4MF specification.
	InG4MFState->G4MFNodes[0]->Transform = FTransform4D::Identity();
	return true;
}

UScene4DComponent* UG4MFDocument4D::ImportGenerateNodeAsComponent(UG4MFState4D* InG4MFState, const int InNodeIndex, AActor* InActor, UScene4DComponent* InSceneParent, UScene4DComponent* InSceneRoot)
{
	if (InG4MFState == nullptr) { return nullptr; }
	if (!InG4MFState->G4MFNodes.IsValidIndex(InNodeIndex))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Invalid node index when generating scene. Aborting file import."));
		return nullptr;
	}
	UG4MFNode4D* g4mfNode = InG4MFState->G4MFNodes[InNodeIndex];
	UScene4DComponent* component = nullptr;
	// First, check if any extension wants to generate a component.
	// If no extension generated a component, check the built-in types.
	if (component == nullptr) {
		component = g4mfNode->ImportGenerateSceneComponent(InG4MFState, InActor);
		if (component == nullptr) {
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to generate a component for a node. Aborting file import."));
			return nullptr;
		}
	}
	// Set up the generated Unreal component.
	g4mfNode->ApplyToSceneComponent(component);
	InG4MFState->UnrealComponents[InNodeIndex] = component;
	// Add the Unreal component to the hierarchy.
	if (InSceneParent)
	{
		component->SetupAttachment(InSceneParent);
	}
	else
	{
		// The root node MUST NOT have a transform as required by the G4MF specification.
		component->SetRelativeTransform(FTransform::Identity);
		InSceneParent = component;
		InSceneRoot = component;
	}
	// Generate children.
	for (int32 childIndex : g4mfNode->ChildrenIndices)
	{
		if (ensure(InG4MFState->G4MFNodes.IsValidIndex(childIndex)))
		{
			ImportGenerateNodeAsComponent(InG4MFState, childIndex, InActor, component, InSceneRoot);
		}
	}
	return component;
}

UMesh4D* UG4MFDocument4D::ImportGenerateCombinedMesh(UG4MFState4D* InG4MFState, UObject* InOuter, const EObjectFlags InObjectFlags, const bool bIncludeInvisible)
{
	if (InG4MFState == nullptr) { return nullptr; }
	const int32 meshCount = InG4MFState->G4MFMeshes.Num();
	const int32 nodeCount = InG4MFState->G4MFNodes.Num();
	UArrayTetraMesh4D* combinedTetraMesh = NewObject<UArrayTetraMesh4D>(InOuter, NAME_None, InObjectFlags);
	for (int32 nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++)
	{
		const UG4MFNode4D* g4mfNode = InG4MFState->G4MFNodes[nodeIndex];
		if (g4mfNode == nullptr)
		{
			continue;
		}
		const UG4MFMeshInstance4D* meshInstance = g4mfNode->MeshInstance;
		if (meshInstance == nullptr)
		{
			continue;
		}
		const int32 meshIndex = meshInstance->MeshIndex;
		if (meshIndex < 0)
		{
			continue;
		}
		if (!(bIncludeInvisible || g4mfNode->bIsVisible))
		{
			continue;
		}
		if (!InG4MFState->G4MFMeshes.IsValidIndex(meshIndex))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Node references an invalid mesh index. Aborting file import."));
			return nullptr;
		}
		// Unreal 4D only supports tetrahedral meshes for now.
		UArrayTetraMesh4D* arrayTetraMesh = nullptr;
		if (InG4MFState->UnrealMeshAssets.IsValidIndex(meshIndex) && IsValid(InG4MFState->UnrealMeshAssets[meshIndex]))
		{
			arrayTetraMesh = Cast<UArrayTetraMesh4D>(InG4MFState->UnrealMeshAssets[meshIndex]);
		}
		else
		{
			const UG4MFMesh4D* g4mfMesh = InG4MFState->G4MFMeshes[meshIndex];
			UMesh4D* mesh = g4mfMesh->ImportGenerateMesh(InG4MFState, InOuter, InObjectFlags);
			InG4MFState->UnrealMeshAssets.SetNum(meshCount);
			InG4MFState->UnrealMeshAssets[meshIndex] = mesh;
			arrayTetraMesh = Cast<UArrayTetraMesh4D>(mesh);
		}
		if (arrayTetraMesh != nullptr)
		{
			combinedTetraMesh->MergeWith(arrayTetraMesh, g4mfNode->GetSceneGlobalTransform(InG4MFState));
		}
	}
	InG4MFState->UnrealCombinedMeshAsset = combinedTetraMesh;
	InG4MFState->bUnrealCombinedMeshAssetIncludeInvisibleSetting = bIncludeInvisible;
	return combinedTetraMesh;
}

// Conversion functions (both import and export).

void UG4MFDocument4D::ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InOrthonormalBasis, const double InAbsScaleFactor)
{
	const FMatrix inverseBasis = UBasis4D::Basis4D_Inverse(InOrthonormalBasis);
	const bool bIsDeterminantNegative = InOrthonormalBasis.Determinant() < 0.0;
	for (TObjectPtr<UG4MFMesh4D>& mesh : InG4MFState->G4MFMeshes)
	{
		mesh->ConvertCoordinateSystem(InG4MFState, InOrthonormalBasis, InAbsScaleFactor, bIsDeterminantNegative);
	}
	if (bIsDeterminantNegative)
	{
		for (TObjectPtr<UG4MFMaterial4D>& material : InG4MFState->G4MFMaterials)
		{
			material->FlipSimplexWindingOrder(InG4MFState);
		}
	}
	for (TObjectPtr<UG4MFNode4D>& node : InG4MFState->G4MFNodes)
	{
		node->Transform.Basis = UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Compose(InOrthonormalBasis, node->Transform.Basis), inverseBasis);
		node->Transform.Origin = InOrthonormalBasis.TransformFVector4(node->Transform.Origin) * InAbsScaleFactor;
		UG4MFCamera4D* camera = node->Camera;
		if (IsValid(camera))
		{
			camera->SizeMeters *= InAbsScaleFactor;
		}
		UG4MFLight4D* light = node->Light;
		if (IsValid(light))
		{
			light->Range *= InAbsScaleFactor;
		}
		UG4MFNodePhysics4D* physics = node->Physics;
		if (IsValid(physics))
		{
			UG4MFNodePhysicsMotion4D* motion = physics->Motion;
			if (IsValid(motion))
			{
				motion->LinearVelocity = InOrthonormalBasis.TransformFVector4(motion->LinearVelocity) * InAbsScaleFactor;
			}
		}
	}
	for (TObjectPtr<UG4MFShape4D>& shape : InG4MFState->G4MFShapes)
	{
		shape->BaseSize = UVectorFuncs4D::Vector4_Abs(InOrthonormalBasis.TransformFVector4(shape->BaseSize) * InAbsScaleFactor);
		shape->RayLength *= InAbsScaleFactor;
		for (TObjectPtr<UGeneralShapeCurve4D>& curve : shape->Curves)
		{
			curve->Radii = UVectorFuncs4D::Vector4_Abs(InOrthonormalBasis.TransformFVector4(curve->Radii) * InAbsScaleFactor);
			for (FGeneralShapeCurveTaperPoint4D& taperPoint : curve->Taper)
			{
				taperPoint.Position = InOrthonormalBasis.TransformFVector4(taperPoint.Position) * InAbsScaleFactor;
				taperPoint.Radii = UVectorFuncs4D::Vector4_Abs(InOrthonormalBasis.TransformFVector4(taperPoint.Radii) * InAbsScaleFactor);
			}
		}
	}
}

// Main export functions.

bool UG4MFDocument4D::ExportAppendFromActorHierarchy(UG4MFState4D* InG4MFState, AActor* InRootActor)
{
	if (InG4MFState == nullptr || InRootActor == nullptr) { return false; }
	UScene4DComponent* rootComponent = Cast<UScene4DComponent>(InRootActor->GetRootComponent());
	if (rootComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Root actor does not have a UScene4DComponent as its root component. Cannot export actor hierarchy as G4MF scene."));
		return false;
	}
	bool success = ExportAppendFromComponentHierarchy(InG4MFState, rootComponent);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to convert actor hierarchy into G4MF state."));
		return false;
	}
	TArray<AActor*> attachedActors;
	InRootActor->GetAttachedActors(attachedActors, true, false);
	// TODO: Handle attached actors (as nodes? as sub-models?).
	return true;
}

bool UG4MFDocument4D::ExportAppendFromComponentHierarchy(UG4MFState4D* InG4MFState, UScene4DComponent* InRootComponent)
{
	if (InG4MFState == nullptr || InRootComponent == nullptr) { return false; }
	InG4MFState->OriginalPath = InRootComponent->GetName();
	bool success = ExportConvertSceneComponent(InG4MFState, InRootComponent, -1);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to convert scene component hierarchy into G4MF state."));
		return false;
	}
	return true;
}

bool UG4MFDocument4D::ExportAppendFromSingleMesh(UG4MFState4D* InG4MFState, UMesh4D* InMesh)
{
	if (InG4MFState == nullptr) { return false; }
	if (InMesh == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Cannot export a null mesh."));
		return false;
	}
	const int32 meshIndex = UG4MFMesh4D::ExportConvertMeshIntoState(InG4MFState, InMesh, true);
	if (meshIndex < 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to convert mesh into G4MF state."));
		return false;
	}
	return true;
}

TArray64<uint8> UG4MFDocument4D::ExportWriteToByteArray(UG4MFState4D* InG4MFState)
{
	if (!ExportSerializeJsonData(InG4MFState))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF data."));
		return TArray64<uint8>();
	}
	return ExportWriteToByteArrayInternal(InG4MFState);
}

bool UG4MFDocument4D::ExportWriteToFile(UG4MFState4D* InG4MFState, const FString& InPath)
{
	if (InG4MFState == nullptr) { return false; }
	// Set up the state with the file path information.
	InG4MFState->G4MFBasePath = FPaths::GetPath(InPath);
	InG4MFState->G4MFFilename = FPaths::GetCleanFilename(InPath);
	InG4MFState->OriginalPath = InPath;
	// Serialize the data to JSON.
	if (!ExportSerializeJsonData(InG4MFState))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF data."));
		return false;
	}
	// Figure out what we're writing.
	const int64 buffer0Size = InG4MFState->G4MFBuffers.Num() > 0 ? InG4MFState->G4MFBuffers[0].Data.Num() : 0;
	const bool bIsTextFile = InG4MFState->IsTextFile();
	const bool bShouldSeparateBuffersIntoFiles = InG4MFState->ShouldSeparateBinaryBlobs(buffer0Size);
	if (bIsTextFile)
	{
		// Write to a G4MF text file. Export the buffers either as base64 or as separate files.
		if (!ExportSerializeBufferData(InG4MFState, bShouldSeparateBuffersIntoFiles))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF buffer data."));
			return false;
		}
		const FString jsonString = ExportSerializeJsonToString(InG4MFState->G4MFJson, true);
		if (!FFileHelper::SaveStringToFile(jsonString, *InPath))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to write JSON data to '%s'."), *InPath);
			return false;
		}
	}
	else
	{
		// Write to a G4MF binary file. Export the buffers as binary blob chunks or as separate files.
		if (bShouldSeparateBuffersIntoFiles)
		{
			if (!ExportSerializeBufferData(InG4MFState, true))
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize G4MF buffer data."));
				return false;
			}
		}
		const TArray64<uint8> jsonBytes = ExportWriteToByteArrayInternal(InG4MFState);
		if (jsonBytes.IsEmpty())
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to serialize binary G4MF data."));
			return false;
		}
		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
		TUniquePtr<IFileHandle> fileHandle(platformFile.OpenWrite(*InPath));
		if (!fileHandle.IsValid() || !fileHandle->Write(jsonBytes.GetData(), jsonBytes.Num()))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to write binary data to '%s'."), *InPath);
			return false;
		}
	}
	return true;
}

// Main import functions.

bool UG4MFDocument4D::ImportReadFromByteArray(UG4MFState4D* InG4MFState, const TArray64<uint8>& InByteArray)
{
	if (InG4MFState == nullptr) { return false; }
	const uint64 byteArraySize = InByteArray.Num();
	if (byteArraySize < 32)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Byte array is too small to be a valid G4MF file."));
		return false;
	}
	const uint8* byteArrayPtr = InByteArray.GetData();
	if (ByteArrayReadUint32(byteArrayPtr, 0) != 0x464D3447) // ASCII string "G4MF" in little-endian
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Byte array is not a valid G4MF file."));
		return false;
	}
	uint64 readOffset = 4;
	if (ByteArrayReadUint32(byteArrayPtr, readOffset) > 0x00000000)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: G4MF file version is not supported."));
		return false;
	}
	readOffset += 4;
	const uint64 declaredFileSize = ByteArrayReadUint64(byteArrayPtr, readOffset);
	if (declaredFileSize < 32)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared file size is too small to contain a valid binary G4MF file."));
		return false;
	}
	if (declaredFileSize > (uint64)INT64_MAX)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared file size exceeds the maximum supported size."));
		return false;
	}
	if (declaredFileSize > byteArraySize)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared file size does not fit within actual file size. File is corrupted."));
		return false;
	}
	readOffset += 8;
	// Read the chunk headers to find where each chunk starts.
	// The first JSON chunk is the G4MF JSON chunk; it does not need to be the first chunk in the file.
	TArray<EG4MFEncodingFormat4D> chunkEncodings;
	TArray64<int64> chunkDataSizes;
	TArray64<int64> chunkDataStarts;
	int64 jsonChunkIndex = -1;
	while (readOffset <= declaredFileSize - 16)
	{
		const uint32 chunkType = ByteArrayReadUint32(byteArrayPtr, readOffset);
		if (jsonChunkIndex == -1 && chunkType == 0x4E4F534A) // ASCII string "JSON" in little-endian
		{
			jsonChunkIndex = chunkDataStarts.Num();
		}
		const uint32 chunkEncodingUint32 = ByteArrayReadUint32(byteArrayPtr, readOffset + 4);
		const EG4MFEncodingFormat4D chunkEncoding = ValidateEncodingFormat(chunkEncodingUint32);
		chunkEncodings.Add(chunkEncoding);
		const uint64 chunkSize = ByteArrayReadUint64(byteArrayPtr, readOffset + 8);
		if (chunkSize > (uint64)INT64_MAX)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Chunk size exceeds the maximum supported size. File is corrupted."));
			return false;
		}
		readOffset += 16;
		if (chunkSize > declaredFileSize - readOffset)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Declared chunk size goes beyond the file size. File is corrupted."));
			return false;
		}
		chunkDataSizes.Add((int64)chunkSize);
		chunkDataStarts.Add((int64)readOffset);
		const uint64 paddedChunkSize = (chunkSize + CHUNK_ALIGNMENT_BITMASK) & ~CHUNK_ALIGNMENT_BITMASK;
		readOffset += paddedChunkSize;
	}
	if (jsonChunkIndex == -1)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: No JSON chunk found in G4MF byte array. File is corrupted."));
		return false;
	}
	// Read the JSON chunk.
	TSharedPtr<FJsonObject> g4mfJson;
	TArray64<int64> bufferChunkIndices;
	TArray64<int64> bufferChunkDeclaredDecodedByteLengths;
	{
		const EG4MFEncodingFormat4D jsonChunkEncoding = chunkEncodings[jsonChunkIndex];
		if (!IsEncodingFormatSupported(jsonChunkEncoding))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: JSON chunk uses an unsupported encoding format."));
			return false;
		}
		const TArray64<uint8> jsonChunk = ImportDecodeChunkData(InByteArray, chunkDataStarts[jsonChunkIndex], chunkDataSizes[jsonChunkIndex], jsonChunkEncoding);
		const FUTF8ToTCHAR jsonText(reinterpret_cast<const ANSICHAR*>(jsonChunk.GetData()), jsonChunk.Num());
		const FString jsonString(jsonText.Length(), jsonText.Get());
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
		bool success = FJsonSerializer::Deserialize(reader, g4mfJson);
		if (!success || !g4mfJson.IsValid())
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse JSON chunk."));
			return false;
		}
		InG4MFState->G4MFJson = g4mfJson;
		// Parse only the JSON structure of the buffers, so we can know which chunks to read and decode.
		success = ImportParseBuffers(InG4MFState, g4mfJson.Get(), &bufferChunkIndices, &bufferChunkDeclaredDecodedByteLengths);
		if (!success)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse G4MF buffers when reading from binary G4MF file at: %s"), *InG4MFState->OriginalPath);
			return false;
		}
	}
	// Read and decode any chunk data needed by a buffer.
	for (int64 bufferIndex = 0; bufferIndex < bufferChunkIndices.Num(); bufferIndex++)
	{
		const int64 chunkIndex = bufferChunkIndices[bufferIndex];
		if (chunkIndex == -1)
		{
			continue;
		}
		if (!chunkDataStarts.IsValidIndex(chunkIndex))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer chunk index %lld is out of range for binary file with %lld chunks. File is corrupted."), chunkIndex, chunkDataStarts.Num());
			return false;
		}
		const EG4MFEncodingFormat4D chunkEncoding = chunkEncodings[chunkIndex];
		if (!IsEncodingFormatSupported(chunkEncoding))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer chunk uses an unsupported encoding format."));
			return false;
		}
		TArray64<uint8> chunkDataDecoded = ImportDecodeChunkData(InByteArray, chunkDataStarts[chunkIndex], chunkDataSizes[chunkIndex], chunkEncoding);
		// The data has been read in now, so check that the size is at least the declared size, and truncate if larger.
		const int64 declaredDecodedByteLength = bufferChunkDeclaredDecodedByteLengths[bufferIndex];
		if (chunkDataDecoded.Num() < declaredDecodedByteLength)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Buffer size is not at least the declared size. Aborting file import."));
			return false;
		}
		chunkDataDecoded.SetNum(declaredDecodedByteLength);
		InG4MFState->G4MFBuffers[bufferIndex].Data = MoveTemp(chunkDataDecoded);
	}
	// Parse the rest of the G4MF JSON data.
	if (!ImportParseJsonData(InG4MFState, g4mfJson.Get()))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse JSON data."));
		return false;
	}
	return true;
}

bool UG4MFDocument4D::ImportReadFromFile(UG4MFState4D* InG4MFState, const FString& InPath)
{
	if (InG4MFState == nullptr) { return false; }
	// Open a file for reading.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TUniquePtr<IFileHandle> FileHandle(PlatformFile.OpenRead(*InPath));
	if (!FileHandle)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to open file for reading."));
		return false;
	}
	const int64 fileSize = FileHandle->Size();
	if (fileSize < 25)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: File is too small to be a valid G4MF file."));
		return false;
	}
	// Set up the state with the file path information.
	InG4MFState->G4MFBasePath = FPaths::GetPath(InPath);
	InG4MFState->G4MFFilename = FPaths::GetCleanFilename(InPath);
	InG4MFState->OriginalPath = InPath;
	// Check for the magic number to allow reading G4MF files regardless of file extension.
	uint32 magicNumberMaybe;
	bool success = FileHandle->Read((uint8*)&magicNumberMaybe, sizeof(uint32));
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to inspect the file header."));
		return false;
	}
	success = FileHandle->Seek(0);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to inspect the file header."));
		return false;
	}
	if (magicNumberMaybe == 0x464D3447) // "G4MF" in little-endian
	{
		TArray64<uint8> fileBytes;
		fileBytes.SetNumUninitialized(fileSize);
		success = FileHandle->Read(fileBytes.GetData(), fileSize);
		if (!success)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to read binary file data."));
			return false;
		}
		return ImportReadFromByteArray(InG4MFState, fileBytes);
	}
	// If there is no magic number, try reading the file as JSON.
	FString jsonString;
	success = FFileHelper::LoadFileToString(jsonString, *InPath);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to read JSON file."));
		return false;
	}
	TSharedPtr<FJsonObject> g4mfJson;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);
	if (!FJsonSerializer::Deserialize(reader, g4mfJson) || !g4mfJson.IsValid())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse JSON from file."));
		return false;
	}
	InG4MFState->G4MFJson = g4mfJson;
	success = ImportParseBuffers(InG4MFState, g4mfJson.Get(), nullptr, nullptr);
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse buffer data."));
		return false;
	}
	success = ImportParseJsonData(InG4MFState, g4mfJson.Get());
	if (!success)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse JSON data."));
		return false;
	}
	return true;
}

UScene4DComponent* UG4MFDocument4D::ImportGenerateComponentHierarchyForActor(UG4MFState4D* InG4MFState, AActor* InActor)
{
	if (InG4MFState == nullptr || InActor == nullptr) { return nullptr; }
	UScene4DComponent *ret = nullptr;
	// Check how many nodes we have, and handle the case of zero nodes.
	if (InG4MFState->G4MFNodes.IsEmpty())
	{
		// If there are no nodes, we can implicitly generate a MeshInstance4D for the first mesh.
		const int32 meshCount = InG4MFState->G4MFMeshes.Num();
		if (meshCount == 0)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: This G4MF file has no nodes or meshes, so it cannot be imported as a scene."));
			return nullptr;
		}
		UG4MFMesh4D* g4mfMesh = InG4MFState->G4MFMeshes[0];
		UMesh4DComponent* meshComponent = NewObject<UMesh4DComponent>(InActor, FName(g4mfMesh->GetItemName()));
		if (InG4MFState->UnrealMeshAssets.IsValidIndex(0) && IsValid(InG4MFState->UnrealMeshAssets[0]))
		{
			meshComponent->Mesh4D = InG4MFState->UnrealMeshAssets[0];
		}
		else
		{
			meshComponent->Mesh4D = g4mfMesh->ImportGenerateMesh(InG4MFState, meshComponent);
			InG4MFState->UnrealMeshAssets.SetNum(meshCount);
			InG4MFState->UnrealMeshAssets[0] = meshComponent->Mesh4D;
			InG4MFState->UnrealCombinedMeshAsset = meshComponent->Mesh4D;
		}
		ret = meshComponent;
	}
	else
	{
		// Prepare the Unreal components array to be filled during scene node generation.
		InG4MFState->UnrealComponents.SetNum(InG4MFState->G4MFNodes.Num());
		// Start from the root node (index 0) which has no parent. This will recursively call itself for child nodes.
		ret = ImportGenerateNodeAsComponent(InG4MFState, 0, InActor, nullptr, nullptr);
	}
	// Clear any references to the Unreal components from the state, since those
	// are only meant to be used during generation and should not persist after.
	InG4MFState->UnrealComponents.Reset();
	return ret;
}

UMesh4D* UG4MFDocument4D::ImportGenerateSingleMesh(UG4MFState4D* InG4MFState, UObject* InOuter, const EObjectFlags InObjectFlags, const int InWhichMeshIndex, const bool bIncludeInvisible)
{
	if (InG4MFState == nullptr) { return nullptr; }
	const int32 meshCount = InG4MFState->G4MFMeshes.Num();
	if (meshCount == 0)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: This G4MF file has no meshes, so it cannot be imported as a mesh."));
		return nullptr;
	}
	// If InWhichMeshIndex is specified, generate that mesh, making no attempt to combine meshes.
	if (InWhichMeshIndex >= 0)
	{
		if (!InG4MFState->G4MFMeshes.IsValidIndex(InWhichMeshIndex))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Specified mesh index is out of range."));
			return nullptr;
		}
		if (InG4MFState->UnrealMeshAssets.IsValidIndex(InWhichMeshIndex) && IsValid(InG4MFState->UnrealMeshAssets[InWhichMeshIndex]))
		{
			return InG4MFState->UnrealMeshAssets[InWhichMeshIndex];
		}
		const UG4MFMesh4D* g4mfMesh = InG4MFState->G4MFMeshes[InWhichMeshIndex];
		UMesh4D* mesh4D = g4mfMesh->ImportGenerateMesh(InG4MFState, InOuter, InObjectFlags);
		InG4MFState->UnrealMeshAssets.SetNum(meshCount);
		InG4MFState->UnrealMeshAssets[InWhichMeshIndex] = mesh4D;
		return mesh4D;
	}
	// Otherwise, if InWhichMeshIndex is -1 or not specified, generate a combined mesh based on the nodes.
	// If there are no nodes, generate the first mesh. We can't combine in this situation because we don't know the mesh transforms.
	if (InG4MFState->G4MFNodes.Num() == 0)
	{
		if (meshCount > 1)
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF import: This G4MF file has multiple meshes, but only the first mesh will be imported."));
		}
		if (InG4MFState->UnrealMeshAssets.IsValidIndex(0) && IsValid(InG4MFState->UnrealMeshAssets[0]))
		{
			return InG4MFState->UnrealMeshAssets[0];
		}
		const UG4MFMesh4D* g4mfMesh = InG4MFState->G4MFMeshes[0];
		UMesh4D* mesh4D = g4mfMesh->ImportGenerateMesh(InG4MFState, InOuter, InObjectFlags);
		InG4MFState->UnrealMeshAssets.SetNum(meshCount);
		InG4MFState->UnrealMeshAssets[0] = mesh4D;
		return mesh4D;
	}
	// If there are nodes, generate a combined mesh.
	if (IsValid(InG4MFState->UnrealCombinedMeshAsset) && bIncludeInvisible == InG4MFState->bUnrealCombinedMeshAssetIncludeInvisibleSetting)
	{
		return InG4MFState->UnrealCombinedMeshAsset;
	}
	UMesh4D* combinedMesh4D = ImportGenerateCombinedMesh(InG4MFState, InOuter, InObjectFlags, bIncludeInvisible);
	InG4MFState->UnrealCombinedMeshAsset = combinedMesh4D;
	InG4MFState->bUnrealCombinedMeshAssetIncludeInvisibleSetting = bIncludeInvisible;
	return combinedMesh4D;
}
