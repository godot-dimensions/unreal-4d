#pragma once

#include "ModelAsset4D.h"
#include "../G4MF/G4MFDocument4D.h"
#include "../G4MF/G4MFState4D.h"

#include "G4MFModelAsset4D.generated.h"

// A 4D model asset imported from a G4MF file (.g4tf / .g4b).
//
// Embeds both the parsed G4MF state and the document (import settings)
// so the asset is fully self-contained at runtime.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFModelAsset4D : public UModelAsset4D
{
	GENERATED_BODY()

public:
	// The fully-parsed G4MF state embedded inside this asset.
	UPROPERTY(VisibleAnywhere, Instanced, Category = "G4MF")
	TObjectPtr<UG4MFState4D> EmbeddedState;

	// The G4MF document that processed the import (preserves settings).
	UPROPERTY(VisibleAnywhere, Instanced, Category = "G4MF")
	TObjectPtr<UG4MFDocument4D> ImportDocument;

	// UModelAsset4D interface.
	virtual UScene4DComponent* GenerateComponentHierarchyForActor(AActor* InActor) override;
	virtual FString GetSourceFormatName() const override;
};
