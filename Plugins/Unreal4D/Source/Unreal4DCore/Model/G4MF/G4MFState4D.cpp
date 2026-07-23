#include "G4MFState4D.h"

void UG4MFState4D::ReserveUniqueNameForSubItem(UG4MFItem4D* InItem, const FString& InUniqueParentName, const FString& InItemSuffix)
{
	if (InItem == nullptr) return;
	FString itemName = InItem->GetItemName();
	if (itemName.IsEmpty())
	{
		itemName = InUniqueParentName + InItemSuffix;
	}
	else if (HasUniqueName(itemName) && FindObject<UObject>(this, *itemName) == InItem && InItem->GetItemName() == itemName)
	{
		// FromJson already reserved this exact object's name before construction.
		// Reserving it again would spuriously append a numeric suffix.
		return;
	}
	itemName = ReserveUniqueName(itemName);
	InItem->SetItemName(this, itemName);
}

void UG4MFState4D::ReserveUniqueNamesForNodeAndSubItems(UG4MFNode4D* InNode, const int32 InNodeIndex)
{
	if (InNode == nullptr) { return; }
	FString nodeName = InNode->GetItemName();
	if (nodeName.IsEmpty())
	{
		nodeName = ReserveUniqueName(FString::Printf(TEXT("Node%d"), InNodeIndex));
		InNode->SetItemName(this, nodeName);
	}
	ReserveUniqueNameForSubItem(InNode->Camera, nodeName, TEXT("Camera"));
	ReserveUniqueNameForSubItem(InNode->Light, nodeName, TEXT("Light"));
	ReserveUniqueNameForSubItem(InNode->MeshInstance, nodeName, TEXT("MeshInstance"));
	ReserveUniqueNameForSubItem(InNode->ModelInstance, nodeName, TEXT("ModelInstance"));
	ReserveUniqueNameForSubItem(InNode->Physics, nodeName, TEXT("Physics"));
}

FName UG4MFState4D::ReserveUniqueFNameFromJson(const FJsonObject* InJson)
{
	if (!InJson->HasField(TEXT("name")))
	{
		return NAME_None;
	}
	const FString rawName = InJson->GetStringField(TEXT("name"));
	if (rawName.IsEmpty())
	{
		return NAME_None;
	}
	return FName(ReserveUniqueName(rawName));
}

int32 UG4MFState4D::AppendG4MFNode(UG4MFNode4D* InNode)
{
	if (!IsValid(InNode))
	{
		return -1;
	}
	const int32 nodeIndex = G4MFNodes.Num();
	ReserveUniqueNamesForNodeAndSubItems(InNode, nodeIndex);
	G4MFNodes.Add(InNode);
	return nodeIndex;
}

FString UG4MFState4D::ReserveUniqueName(const FString& InRequestedName)
{
	const FString sanitizedRequestedName = UG4MFItem4D::SanitizeItemName(InRequestedName);
	FString uniqueName = sanitizedRequestedName;
	if (_uniqueNames.Contains(uniqueName))
	{
		uint64 discriminator = 2;
		// We need to check Unreal's object names as well, because even if the name is not in _uniqueNames,
		// it may have a case-insensitive conflict with an existing UObject name, which are case-insensitive.
		while (_uniqueNames.Contains(uniqueName) || FindObject<UObject>(this, *uniqueName) != nullptr)
		{
			uniqueName = sanitizedRequestedName + FString::FromInt(discriminator);
			discriminator++;
		}
		UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MFState4D: The requested name '%s' is already in use. Using '%s' instead."), *InRequestedName, *uniqueName);
	}
	else if (!InRequestedName.Equals(sanitizedRequestedName)) // Only happens when sanitizedRequestedName also equals uniqueName.
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MFState4D: The requested name '%s' was sanitized to '%s'."), *InRequestedName, *sanitizedRequestedName);
	}
	_uniqueNames.Add(uniqueName);
	return uniqueName;
}

bool UG4MFState4D::UnreserveUniqueName(const FString& InName)
{
	return _uniqueNames.Remove(InName) > 0;
}

bool UG4MFState4D::IsTextFile() const
{
	// Checking length > 3 handles "g4tf", "g4mf", "json", "g4tf.json", "g4mf.json", etc.
	const FString extension = FPaths::GetExtension(G4MFFilename);
	return extension.Len() > 3;
}

bool UG4MFState4D::ShouldSeparateBinaryBlobs(const int64 InBlobSize) const
{
	if (ExternalDataMode == EG4MFExternalDataMode4D::SeparateAllFiles ||
		ExternalDataMode == EG4MFExternalDataMode4D::SeparateBinaryBlobs)
	{
		if (G4MFBasePath.IsEmpty())
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF: No base path is set, cannot separate binary blob files."));
			return false;
		}
		if (FPaths::GetBaseFilename(G4MFFilename).IsEmpty())
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF: No filename is set, cannot separate binary blob files."));
			return false;
		}
		return true;
	}
	if (ExternalDataMode == EG4MFExternalDataMode4D::EmbedEverything ||
		ExternalDataMode == EG4MFExternalDataMode4D::SeparateResourceFiles)
	{
		return false;
	}
	// EG4MFExternalDataMode4D::Automatic embeds everything for binary files (.g4b),
	// byte arrays in memory (no extension), when there is no base path set,
	// and when the size is below an arbitrary threshold (we'll use 2 KiB),
	// but separates for text files (.g4tf) when the base path and filename are valid.
	if (InBlobSize < 2048)
	{
		return false;
	}
	return IsTextFile() && !G4MFBasePath.IsEmpty() && !FPaths::GetBaseFilename(G4MFFilename).IsEmpty();
}

bool UG4MFState4D::ShouldSeparateResourceFiles() const
{
	if (ExternalDataMode == EG4MFExternalDataMode4D::SeparateAllFiles ||
		ExternalDataMode == EG4MFExternalDataMode4D::SeparateResourceFiles)
	{
		if (G4MFBasePath.IsEmpty())
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF: No base path is set, cannot separate resource files."));
			return false;
		}
		if (FPaths::GetBaseFilename(G4MFFilename).IsEmpty())
		{
			UE_LOG(LogUnreal4DCore, Warning, TEXT("G4MF: No filename is set, cannot separate resource files."));
			return false;
		}
		return true;
	}
	if (ExternalDataMode == EG4MFExternalDataMode4D::EmbedEverything ||
		ExternalDataMode == EG4MFExternalDataMode4D::SeparateBinaryBlobs)
	{
		return false;
	}
	return IsTextFile() && !G4MFBasePath.IsEmpty() && !FPaths::GetBaseFilename(G4MFFilename).IsEmpty();
}

bool UG4MFState4D::ShouldSeparateModelFiles() const
{
	// For model files, we separate them in all modes except when embedding everything or if
	// we are forced to embed everything because the base path is empty (nowhere to put files).
	if (ExternalDataMode == EG4MFExternalDataMode4D::EmbedEverything)
	{
		return false;
	}
	return !G4MFBasePath.IsEmpty();
}
