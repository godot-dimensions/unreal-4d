#pragma once

#include "../../Unreal4DDefines.h"

#include "GameFramework/Actor.h"

#include "ModelActor4D.generated.h"

class UModelAsset4D;
class UScene4DComponent;
class UActorComponent;

// An Actor that represents a placed 4D model in a Level.
//
// Holds a reference to a UModelAsset4D and generates its component
// hierarchy at editor-time on placement, duplication, and model change.
// For saved levels, the hierarchy is deserialized alongside the actor
// (as InstanceComponents) and does not need to be regenerated.
//
// The generated root component becomes the Actor's RootComponent,
// so code can treat GetRootComponent() as the model root without
// special-casing.
UCLASS(Category = "Unreal 4D|Model")
class UNREAL4DCORE_API AModelActor4D : public AActor
{
	GENERATED_BODY()

private:
	// The root of the currently generated component hierarchy.
	// Serialized (not Transient) so that after loading a saved level,
	// the OnConstruction guard can see that the hierarchy is already
	// intact and skip an unnecessary RebuildFromAsset. Without this,
	// the guard would see null and tear down the deserialized components
	// (losing user-modified 4D transforms) to rebuild from the asset.
	UPROPERTY()
	TObjectPtr<UScene4DComponent> GeneratedRoot;

	// The model asset that was last used to build the hierarchy.
	// Serialized alongside GeneratedRoot so the OnConstruction guard
	// can confirm the hierarchy matches the current Model after load.
	UPROPERTY()
	TObjectPtr<UModelAsset4D> LastBuiltModel;

	// Components created by the most recent model build. Tracking these
	// explicitly keeps rebuilds from destroying components added by a
	// Blueprint subclass or by user code.
	UPROPERTY()
	TArray<TObjectPtr<UActorComponent>> GeneratedComponents;

	void DestroyGeneratedComponents();

public:
	AModelActor4D();

	// The model asset this actor is visualising.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Model")
	TObjectPtr<UModelAsset4D> Model;

	// Tear down the current component hierarchy and regenerate it
	// from the assigned Model asset.
	UFUNCTION(BlueprintCallable, Category = "Model")
	void RebuildFromAsset();

	// AActor overrides.
	virtual void OnConstruction(const FTransform& InTransform) override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) override;
#endif
};
