#pragma once

#include "Scene4DActor.h"
#include "../Components/Camera4DComponent.h"

#include "Camera4DActor.generated.h"

UCLASS(BlueprintType, Category = "Unreal 4D|Actor|Camera")
class ACamera4DActor : public AScene4DActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera 4D")
	TObjectPtr<UCamera4DComponent> Camera4DComponent = nullptr;

	ACamera4DActor(const FObjectInitializer& Init);
};
