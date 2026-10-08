#pragma once

#include "../../Unreal4DEditorDefines.h"

#include "ActorFactories/ActorFactory.h"

#include "G4MFModelActor4DFactory.generated.h"

// Actor factory that spawns AModelActor4D when a
// UG4MFModelAsset4D is dragged into a level.
UCLASS()
class UG4MFModelActor4DFactory : public UActorFactory
{
	GENERATED_BODY()

public:
	UG4MFModelActor4DFactory();

	// UActorFactory interface
	virtual bool CanCreateActorFrom(const FAssetData& InAssetData, FText& OutErrorMsg) override;
	virtual UClass* GetDefaultActorClass(const FAssetData& InAssetData) override;
	virtual bool CanPlaceElementsFromAssetData(const FAssetData& InAssetData) override;

protected:
	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;
};
