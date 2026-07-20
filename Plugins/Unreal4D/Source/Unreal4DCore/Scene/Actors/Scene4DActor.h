#pragma once

#include "../Components/Scene4DComponent.h"

#include "Scene4DActor.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Actor")
class UNREAL4DCORE_API AScene4DActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unreal 4D|Actor")
	TObjectPtr<UScene4DComponent> Root4DComponent = nullptr;

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Actor")
	UScene4DComponent* GetSelf4DComponent() const;

	AScene4DActor(const FObjectInitializer& Init);
};
