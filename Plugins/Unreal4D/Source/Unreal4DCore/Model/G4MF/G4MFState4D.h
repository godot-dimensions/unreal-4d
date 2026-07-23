#pragma once

#include "Structures/Physics/G4MFShape4D.h"
#include "Structures/G4MFAccessor4D.h"
#include "Structures/G4MFBufferView4D.h"
#include "Structures/G4MFMaterial4D.h"
#include "Structures/G4MFMesh4D.h"
#include "Structures/G4MFModel4D.h"
#include "Structures/G4MFNode4D.h"
#include "Structures/G4MFTexture4D.h"

#include "G4MFState4D.generated.h"

// UG4MFState4D contains all nodes and resources of a G4MF file. This is used by
// UG4MFDocument4D as data storage, which allows UG4MFDocument4D to be stateless.
// When importing a G4MF file, a UG4MFState4D can be populated by UG4MFDocument4D by
// reading a file. This data can then be used by UG4MFDocument4D to create an Unreal actor or a mesh resource.
// When exporting an Unreal actor or mesh, a G4MFState4D can be populated by UG4MFDocument4D by
// converting the scene. This data can then be used by UG4MFDocument4D to save to a G4MF file.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFState4D : public UG4MFItem4D
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TSet<FString> _uniqueNames;

public:
	UPROPERTY()
	TArray<TObjectPtr<UG4MFAccessor4D>> G4MFAccessors;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFBufferView4D>> G4MFBufferViews;

	UPROPERTY()
	TArray<FG4MFBuffer4D> G4MFBuffers;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFFileReference4D>> G4MFFiles;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFTexture4D>> G4MFTextures;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFMaterial4D>> G4MFMaterials;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFMesh4D>> G4MFMeshes;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFShape4D>> G4MFShapes;

	UPROPERTY()
	TArray<TObjectPtr<UG4MFNode4D>> G4MFNodes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UScene4DComponent>> UnrealComponents;

	// Individual meshes generated from the G4MF file saved into separate .uasset files for easier referencing.
	UPROPERTY()
	TArray<TObjectPtr<UMesh4D>> UnrealMeshAssets;

	UPROPERTY()
	TObjectPtr<UMesh4D> UnrealCombinedMeshAsset;

	UPROPERTY()
	bool bUnrealCombinedMeshAssetIncludeInvisibleSetting = false;

	TSharedPtr<FJsonObject> G4MFJson;

	UPROPERTY()
	int32 DeclaredDimension = 4;

	// Path data for the file.
	UPROPERTY()
	FString G4MFBasePath = TEXT("");

	UPROPERTY()
	FString G4MFFilename = TEXT("");

	UPROPERTY()
	FString OriginalPath = TEXT("");

	// Settings for handling the file.
	UPROPERTY()
	EG4MFExternalDataMode4D ExternalDataMode = EG4MFExternalDataMode4D::Automatic;

	void ReserveUniqueNameForSubItem(UG4MFItem4D* InItem, const FString& InUniqueParentName, const FString& InItemSuffix);
	void ReserveUniqueNamesForNodeAndSubItems(UG4MFNode4D* InNode, const int32 InNodeIndex);
	FName ReserveUniqueFNameFromJson(const FJsonObject* InJson);
	int32 AppendG4MFNode(UG4MFNode4D* InNode);

	bool HasUniqueName(const FString& InName) const { return _uniqueNames.Contains(InName); }
	FString ReserveUniqueName(const FString& InRequestedName);
	bool UnreserveUniqueName(const FString& InName);

	bool IsTextFile() const;
	bool ShouldSeparateBinaryBlobs(const int64 InBlobSize) const;
	bool ShouldSeparateResourceFiles() const;
	bool ShouldSeparateModelFiles() const;
};
