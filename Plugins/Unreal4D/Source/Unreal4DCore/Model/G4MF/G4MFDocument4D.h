#pragma once

#include "G4MFState4D.h"

#include "G4MFDocument4D.generated.h"

class UScene4DComponent;

// UG4MFDocument4D contains functions for importing and exporting G4MF files.
// It uses a UG4MFState4D as data storage, which allows UG4MFDocument4D to be stateless.
// When importing a G4MF file, a UG4MFState4D can be populated by UG4MFDocument4D by
// reading a file. This data can then be used to create an Unreal actor or a mesh resource.
// When exporting from Unreal, a UG4MFState4D can be populated by UG4MFDocument4D by
// converting the actors, components, meshes, etc. This data can then be used to save to a G4MF file.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFDocument4D : public UObject
{
	GENERATED_BODY()

private:
	static constexpr uint64 CHUNK_ALIGNMENT_BITMASK = (uint64)15;

	// Byte array helper functions (abstractions using `FMemory::Memcpy` for cross-platform safety).
	static uint32 ByteArrayReadUint32(const uint8* InByteArrayPtr, const uint64 InOffset);
	static uint64 ByteArrayReadUint64(const uint8* InByteArrayPtr, const uint64 InOffset);
	// File helper functions (abstractions on top of limited Unreal IFileHandle API).
	static uint32 FileReadUint32(const TUniquePtr<IFileHandle>& InFileHandle);
	static uint64 FileReadUint64(const TUniquePtr<IFileHandle>& InFileHandle);

	EG4MFEncodingFormat4D ValidateEncodingFormat(const uint32 InEncodingFormatUint32);
	bool IsEncodingFormatSupported(const EG4MFEncodingFormat4D InEncodingFormat);
	FString Uint32ToAsciiString(uint32 InValue, const bool bInAllowAndEscapeNonAscii);
	uint32 AsciiStringToUint32(const FString& InValue);

	// Export process (in order of operations).
	bool ExportConvertSceneComponent(UG4MFState4D* InG4MFState, UScene4DComponent* InComponent, const int32 InParentIndex);
	bool ExportSerializeJsonData(UG4MFState4D* InG4MFState);
	bool ExportSerializeNodes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeShapes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeMeshes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeMaterials(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeTextures(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeFiles(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeBuffersAccessors(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ExportSerializeBufferData(UG4MFState4D* InG4MFState, const bool InShouldSeparateBuffersIntoFiles);
	bool ExportSerializeAssetHeader(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	static FString ExportSerializeJsonToString(TSharedPtr<FJsonObject>& InG4MFJson, const bool bInPrettyPrint);
	TArray64<uint8> ExportEncodeChunkData(UG4MFState4D* InG4MFState, const TArray64<uint8>& InBufferData);
	TArray64<uint8> ExportWriteToByteArrayInternal(UG4MFState4D* InG4MFState);

	// Import process (in order of operations).
	bool ImportReadFromBinaryFile(UG4MFState4D* InG4MFState, const TUniquePtr<IFileHandle>& InFileHandle);
	TArray64<uint8> ImportDecodeChunkData(const TArray64<uint8>& InFileOrChunkData, const int64 InChunkDataOffset, const int64 InChunkDataRawSize, const EG4MFEncodingFormat4D InChunkEncodingFormat);
	bool ImportParseBuffers(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson, TArray64<int64>* OutBufferChunkIndices, TArray64<int64>* OutBufferChunkDeclaredDecodedByteLengths);
	bool ImportParseJsonData(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseNodeNames(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseAssetHeader(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseBufferViews(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseAccessors(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseFiles(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseTextures(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseMaterials(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseMeshes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseShapes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	bool ImportParseNodes(UG4MFState4D* InG4MFState, FJsonObject* InG4MFJson);
	UScene4DComponent* ImportGenerateNodeAsComponent(UG4MFState4D* InG4MFState, const int InNodeIndex, AActor* InActor, UScene4DComponent* InSceneParent, UScene4DComponent* InSceneRoot);
	UMesh4D* ImportGenerateCombinedMesh(UG4MFState4D* InG4MFState, UObject* InOuter, const EObjectFlags InObjectFlags = RF_NoFlags, const bool InIncludeInvisible = false);

public:
	UPROPERTY()
	EG4MFEncodingFormat4D EncodingFormat = EG4MFEncodingFormat4D::Plain;

	// -1 means unlimited depth.
	UPROPERTY()
	int32 MaxNestedSceneDepth = -1;

	// Conversion functions (both import and export).
	void ConvertCoordinateSystem(UG4MFState4D* InG4MFState, const FMatrix& InBasis, const double InScaleFactor);

	// Main export functions.
	bool ExportAppendFromActorHierarchy(UG4MFState4D* InG4MFState, AActor* InRootActor);
	bool ExportAppendFromComponentHierarchy(UG4MFState4D* InG4MFState, UScene4DComponent* InRootComponent);
	bool ExportAppendFromSingleMesh(UG4MFState4D* InG4MFState, UMesh4D* InMesh);
	TArray64<uint8> ExportWriteToByteArray(UG4MFState4D* InG4MFState);
	bool ExportWriteToFile(UG4MFState4D* InG4MFState, const FString& InPath);

	// Main import functions.
	bool ImportReadFromByteArray(UG4MFState4D* InG4MFState, const TArray64<uint8>& InByteArray);
	bool ImportReadFromFile(UG4MFState4D* InG4MFState, const FString& InPath);
	UScene4DComponent* ImportGenerateComponentHierarchyForActor(UG4MFState4D* InG4MFState, AActor* InActor);
	UMesh4D* ImportGenerateSingleMesh(UG4MFState4D* InG4MFState, UObject* InOuter, const EObjectFlags InObjectFlags = RF_NoFlags, const int InWhichMeshIndex = -1, const bool InIncludeInvisible = false);
};
