#include "G4MFModelActor4DFactory.h"

#include "../../../Unreal4DCore/Model/Asset/G4MFModelAsset4D.h"
#include "../../../Unreal4DCore/Model/Asset/ModelActor4D.h"
#include "../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"

UG4MFModelActor4DFactory::UG4MFModelActor4DFactory()
{
	DisplayName = FText::FromString(TEXT("G4MF Model 4D"));
	NewActorClass = AModelActor4D::StaticClass();
}

bool UG4MFModelActor4DFactory::CanCreateActorFrom(const FAssetData& InAssetData, FText& OutErrorMsg)
{
	if (!InAssetData.IsValid())
	{
		OutErrorMsg = INVTEXT("The asset data was not valid.");
		return false;
	}
	if (!InAssetData.IsInstanceOf(UG4MFModelAsset4D::StaticClass()))
	{
		OutErrorMsg = INVTEXT("The asset data was not a G4MFModelAsset4D.");
		return false;
	}
	return true;
}

UClass* UG4MFModelActor4DFactory::GetDefaultActorClass(const FAssetData& InAssetData)
{
	return AModelActor4D::StaticClass();
}

bool UG4MFModelActor4DFactory::CanPlaceElementsFromAssetData(const FAssetData& InAssetData)
{
	return InAssetData.IsValid() && InAssetData.IsInstanceOf(UG4MFModelAsset4D::StaticClass());
}

AActor* UG4MFModelActor4DFactory::SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams)
{
	UG4MFModelAsset4D* modelAsset = Cast<UG4MFModelAsset4D>(InAsset);
	if (modelAsset == nullptr)
	{
		return nullptr;
	}
	ULevel* spawnLevel = ValidateSpawnActorLevel(InLevel, InSpawnParams);
	if (spawnLevel == nullptr)
	{
		return nullptr;
	}
	// Use deferred construction (bDeferConstruction) so we can assign Model
	// before OnConstruction fires. Without this, SpawnActor internally calls
	// FinishSpawning → ExecuteConstruction → OnConstruction with Model still
	// null, wasting a full teardown+rebuild cycle. With deferred construction,
	// SpawnActor returns immediately after constructing the UObject, letting
	// us set properties before manually calling FinishSpawning.
	FActorSpawnParameters params = InSpawnParams;
	params.OverrideLevel = spawnLevel;
	params.bDeferConstruction = true;
	AModelActor4D* newActor = spawnLevel->GetWorld()->SpawnActor<AModelActor4D>(AModelActor4D::StaticClass(), InTransform, params);
	if (newActor == nullptr)
	{
		UE_LOG(LogUnreal4DEditor, Error, TEXT("UG4MFModelActor4DFactory: Failed to spawn AModelActor4D."));
		return nullptr;
	}
	newActor->Model = modelAsset;
	// FinishSpawning runs ExecuteConstruction → OnConstruction → RebuildFromAsset
	// with Model already set, producing a single real rebuild. The editor's
	// placement pipeline (CreateActor/PostPlaceAsset) then fires PostEditChange
	// and PostEditMove, which re-trigger OnConstruction several more times -
	// the idempotency guard in OnConstruction skips these since the hierarchy
	// is already up-to-date.
	newActor->FinishSpawning(InTransform);
	// Use the authoritative placement transform provided by the editor's placement
	// pipeline to initialize the actor's 4D position at drop location.
	UScene4DComponent* root4D = Cast<UScene4DComponent>(newActor->GetRootComponent());
	ensure(IsValid(root4D)); // UG4MFModelActor4DFactory should only produce actors with 4D root components.
	root4D->SetLocalTransform4Dfrom3D(InTransform);
	return newActor;
}
