#include "G4MFModelAsset4D.h"

UScene4DComponent* UG4MFModelAsset4D::GenerateComponentHierarchyForActor(AActor* InActor)
{
	if (!IsValid(EmbeddedState) || !IsValid(ImportDocument))
	{
		return nullptr;
	}
	return ImportDocument->ImportGenerateComponentHierarchyForActor(EmbeddedState, InActor);
}

FString UG4MFModelAsset4D::GetSourceFormatName() const
{
	return TEXT("G4MF");
}
