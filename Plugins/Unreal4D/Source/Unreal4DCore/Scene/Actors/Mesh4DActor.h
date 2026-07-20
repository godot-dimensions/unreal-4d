#pragma once

#include "../Components/Mesh4DComponent.h"

#include "Mesh4DActor.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Actor|Mesh")
class AMesh4DActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Mesh 4D")
	TObjectPtr<UMesh4DComponent> Mesh4DComponent = nullptr;

private:
	UPROPERTY(meta = (DeprecatedProperty))
	TObjectPtr<UScene4DComponent> Root4DComponent = nullptr;

	UPROPERTY()
	bool UpgradedRoot4DComponent = false;

	UPROPERTY(meta = (DeprecatedProperty))
	TObjectPtr<UMesh4DComponent> Mesh4DComponentGPU = nullptr;

public:
	AMesh4DActor(const FObjectInitializer& Init);

protected:
	//~ AActor BEGIN

	void PostLoad() override;

	//~ AActor END
};
