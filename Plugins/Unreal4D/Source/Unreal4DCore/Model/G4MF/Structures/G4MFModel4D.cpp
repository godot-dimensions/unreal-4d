#include "G4MFModel4D.h"

#include "../../../Scene/Components/Scene4DComponent.h"
#include "../../../Scene/Components/Mesh4DComponent.h"
#include "../G4MFDocument4D.h"

FString UG4MFModel4D::GetInferredMimeType() const
{
	if (!MimeType.IsEmpty())
	{
		return MimeType;
	}
	// If MIME type is not set, try to guess based on file extension.
	const FString extension = FPaths::GetExtension(FileUri, false).ToLower();
	if (extension == "g4b")
	{
		return "model/g4mf-binary";
	}
	else if (extension == "g4tf")
	{
		return "model/g4mf+json";
	}
	else if (extension == "off")
	{
		return "model/off";
	}
	else if (extension == "glb")
	{
		return "model/gltf-binary";
	}
	else if (extension == "gltf")
	{
		return "model/gltf+json";
	}
	else if (extension == "fbx")
	{
		return "model/fbx";
	}
	else if (extension == "obj")
	{
		return "model/obj";
	}
	return FString();
}

bool UG4MFModel4D::ImportParseFileData(UG4MFState4D* InG4MFState)
{
	if (InG4MFState == nullptr) { return false; }
	const FString inferredMimeType = GetInferredMimeType();
	if (inferredMimeType.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: MIME type is empty."));
		return false;
	}
	// Handle based on declared or inferred MIME type.
	if (inferredMimeType == TEXT("model/g4mf-binary") || inferredMimeType == TEXT("model/g4mf+json"))
	{
		if (!IsValid(ModelG4MFDocument))
		{
			ModelG4MFDocument = NewObject<UG4MFDocument4D>(this);
		}
		ModelG4MFState = NewObject<UG4MFState4D>(this);
		ModelG4MFState->ExternalDataMode = InG4MFState->ExternalDataMode;
		if (FileUri.IsEmpty() || FileUri.StartsWith(TEXT("data:")))
		{
			// Embedded models resolve any of their own relative external
			// references beside the containing G4MF file, unless overridden.
			ModelG4MFState->G4MFBasePath = InG4MFState->G4MFBasePath;
			const TArray64<uint8> fileData = ReadFileBytes(InG4MFState);
			if (fileData.IsEmpty())
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: Failed to read model file data."));
				return false;
			}
			const bool byteArraySuccess = ModelG4MFDocument->ImportReadFromByteArray(ModelG4MFState, fileData);
			if (!byteArraySuccess)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse embedded G4MF model data."));
				return false;
			}
		}
		else
		{
			const FString fullFilePath = FPaths::Combine(InG4MFState->G4MFBasePath, FileUri);
			const bool fileSuccess = ModelG4MFDocument->ImportReadFromFile(ModelG4MFState, fullFilePath);
			if (!fileSuccess)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: Failed to parse referenced G4MF model '%s'."), *fullFilePath);
				return false;
			}
		}
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: Unsupported model MIME type: %s"), *MimeType);
		return false;
	}
	return true;
}

UScene4DComponent* UG4MFModel4D::ImportInstantiateModel(const UG4MFState4D* InG4MFState, AActor* InActor)
{
	if (InG4MFState == nullptr || InActor == nullptr) { return nullptr; }
	const FString inferredMimeType = GetInferredMimeType();
	if (inferredMimeType.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: MIME type is empty."));
		return nullptr;
	}
	if (!IsValid(ModelG4MFDocument))
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: ModelG4MFDocument is not valid."));
		return nullptr;
	}
	UScene4DComponent* ret = nullptr;
	int maxNestedSceneDepth = ModelG4MFDocument->MaxNestedSceneDepth;
	// Handle based on declared or inferred MIME type.
	if (inferredMimeType == "model/g4mf-binary" || inferredMimeType == "model/g4mf+json")
	{
		if (!IsValid(ModelG4MFState))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: ModelG4MFState is not valid."));
			return nullptr;
		}
		ModelG4MFState->ExternalDataMode = InG4MFState->ExternalDataMode;
		UG4MFDocument4D* modelDocument = ModelG4MFDocument;
		if (maxNestedSceneDepth > 0)
		{
			modelDocument = DuplicateObject<UG4MFDocument4D>(ModelG4MFDocument, this);
			modelDocument->MaxNestedSceneDepth = maxNestedSceneDepth - 1;
		}
		const int32 modelNodeCount = ModelG4MFState->G4MFNodes.Num();
		if (modelNodeCount == 0)
		{
			// If the G4MF file does not contain any nodes, create one to hold the mesh data, if any.
			const int32 modelMeshCount = ModelG4MFState->G4MFMeshes.Num();
			if (modelMeshCount == 0)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF import: UG4MFModel4D: Referenced G4MF file contains no nodes or meshes."));
				return nullptr;
			}
			// Note: The name will be reassigned later by the caller. But we should still start
			// with something meaningful, even if it's just a fallback inferred from the URI.
			FString modelBaseName = GetItemName();
			if (modelBaseName.IsEmpty() && !FileUri.IsEmpty())
			{
				modelBaseName = FPaths::GetBaseFilename(FileUri);
			}
			// Generate a root 4D component and a mesh instance child component.
			const FName rootName = MakeUniqueObjectName(InActor, UScene4DComponent::StaticClass(), FName(GetItemName()));
			ret = NewObject<UScene4DComponent>(InActor, rootName);
			const FName meshCompName = MakeUniqueObjectName(InActor, UMesh4DComponent::StaticClass(), FName(rootName.ToString() + TEXT("Mesh")));
			UMesh4DComponent* meshComponent = NewObject<UMesh4DComponent>(InActor, meshCompName);
			// Generate the mesh data for the single mesh, and link up everything.
			meshComponent->Mesh4D = modelDocument->ImportGenerateSingleMesh(ModelG4MFState, meshComponent);
			meshComponent->SetupAttachment(ret);
		}
		else
		{
			// Normal case - generate the component hierarchy based on the nodes in the G4MF file.
			ret = modelDocument->ImportGenerateComponentHierarchyForActor(ModelG4MFState, InActor);
		}
	}
	else if (inferredMimeType == "model/off")
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF import: UG4MFModel4D: OFF model instantiation not yet implemented."));
	}
	return ret;
}

int32 UG4MFModel4D::ExportPackNodesIntoModel(UG4MFDocument4D* InG4MFDocument, UG4MFState4D* InG4MFState, UScene4DComponent* InRootComponent, const bool bInDeduplicate)
{
	if (InG4MFDocument == nullptr || InG4MFState == nullptr || InRootComponent == nullptr) { return -1; }
	UG4MFDocument4D* modelDocument = InG4MFDocument;
	const int32 maxNestedSceneDepth = modelDocument->MaxNestedSceneDepth;
	if (maxNestedSceneDepth > 0)
	{
		modelDocument = DuplicateObject<UG4MFDocument4D>(InG4MFDocument, modelDocument->GetOuter());
		modelDocument->MaxNestedSceneDepth = maxNestedSceneDepth - 1;
	}
	// Create a new G4MFModel4D to hold a reference to the model data.
	UG4MFModel4D* g4mfModel = NewObject<UG4MFModel4D>(InG4MFState);
	g4mfModel->ModelG4MFDocument = modelDocument;
	g4mfModel->FileUri = InRootComponent->GetName();
	// Fill the G4MFModel4D with the model data.
	{
		UG4MFState4D* modelG4MFState = NewObject<UG4MFState4D>(g4mfModel);
		modelG4MFState->ExternalDataMode = InG4MFState->ExternalDataMode;
		if (!modelDocument->ExportAppendFromComponentHierarchy(modelG4MFState, InRootComponent))
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to append Unreal scene data into G4MF document."));
			return -1;
		}
		// Ensure that the root node is untransformed and visible (if invisible,
		// the *instance* should be the one to be invisible, not the model itself).
		{
			const int32 modelNodeCount = modelG4MFState->G4MFNodes.Num();
			if (modelNodeCount == 0)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to export Unreal scene to G4MF document, no nodes were generated."));
				return -1;
			}
			UG4MFNode4D* rootNode = modelG4MFState->G4MFNodes[0];
			if (rootNode == nullptr)
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to export Unreal scene to G4MF document, root node is null."));
				return -1;
			}
			rootNode->Transform = FTransform4D::Identity();
			rootNode->bIsVisible = true;
		}
		g4mfModel->ModelG4MFState = modelG4MFState;
	}
	// Append the model file reference to the state.
	const int32 modelFileIndex = InG4MFState->G4MFFiles.Num();
	InG4MFState->G4MFFiles.Add(g4mfModel);
	return modelFileIndex;
}

bool UG4MFModel4D::ExportSerializeFileData(UG4MFState4D* InG4MFState, const bool bInDeduplicate, const int32 InBufferIndex)
{
	if (InG4MFState == nullptr || ModelG4MFDocument == nullptr) { return false; }
	const bool bIsTextFile = InG4MFState->IsTextFile();
	const bool bShouldSeparateModelFiles = InG4MFState->ShouldSeparateModelFiles();
	// Determine the file extension and MIME type to use based on what data is available.
	FString targetExtension;
	if (IsValid(ModelG4MFState))
	{
		targetExtension = bIsTextFile ? TEXT(".g4tf") : TEXT(".g4b");
		MimeType = bIsTextFile ? TEXT("model/g4mf+json") : TEXT("model/g4mf-binary");
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: No model data to write."));
		return false;
	}
	// Use the MIME type determined above as-is, and use the file extension to determine the path and name.
	FString targetUri = FileUri;
	if (targetUri.IsEmpty())
	{
		targetUri = GetItemName();
		if (targetUri.IsEmpty())
		{
			int64 indexOfSelf = InG4MFState->G4MFFiles.IndexOfByKey(this);
			targetUri = FString::Printf(TEXT("Model%" INT64_FMT), indexOfSelf);
		}
	}
	if (!targetUri.EndsWith(targetExtension))
	{
		targetUri += targetExtension;
	}
	const FString fullFilePath = FPaths::Combine(InG4MFState->G4MFBasePath, targetUri);
	if (!bShouldSeparateModelFiles || targetUri.Contains(TEXT("/")))
	{
		SetItemName(InG4MFState, InG4MFState->ReserveUniqueName(FPaths::GetCleanFilename(targetUri)));
	}
	// If we are writing to a separate model file, set the URI and write to that location.
	if (bShouldSeparateModelFiles)
	{
		FileUri = targetUri;
		const bool dirSuccess = CreateMissingDirectoriesIfNeeded(InG4MFState);
		if (!dirSuccess)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to create directories for nested model '%s'."), *targetUri);
			return false;
		}
		if (IsValid(ModelG4MFState))
		{
			return ModelG4MFDocument->ExportWriteToFile(ModelG4MFState, fullFilePath);
		}
		else
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: No model data to write, but should've exited earlier."));
			return false;
		}
	}
	// Embed the model data into a buffer view. The URI must be empty in this case.
	FileUri = TEXT("");
	TArray64<uint8> modelBytes;
	if (IsValid(ModelG4MFState))
	{
		modelBytes = ModelG4MFDocument->ExportWriteToByteArray(ModelG4MFState);
	}
	else
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: No model data to write, but should've exited earlier."));
		return false;
	}
	// Now that we have the bytes, write them into a new buffer view in the state.
	if (modelBytes.IsEmpty())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: The exported model data is an empty byte array, export failed."));
		return false;
	}
	// Always use 16-byte alignment for models to allow 16-byte headers to be aligned.
	const int32 bufferViewIndex = UG4MFBufferView4D::WriteNewBufferViewIntoState(InG4MFState, modelBytes, 16, bInDeduplicate, InBufferIndex);
	if (bufferViewIndex < 0)	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("G4MF export: Failed to write model data into a new buffer view in the state."));
		return false;
	}
	BufferViewIndex = bufferViewIndex;
	return true;
}

UG4MFModel4D* UG4MFModel4D::FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson)
{
	// General code for all UG4MFItem4D-derived classes.
	if (InJson == nullptr) { return nullptr; }
	const FName uniqueName = InG4MFState->ReserveUniqueFNameFromJson(InJson);
	UG4MFModel4D* ret = NewObject<UG4MFModel4D>(InG4MFState, uniqueName);
	ret->ReadFileReferenceEntriesFromJson(InJson);
	// UG4MFModel4D-specific code (nothing).
	return ret;
}
