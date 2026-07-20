#pragma once

#include "Scene4DComponent.h"

#include "Primitive4DComponent.generated.h"

// Base class for 4D components that represent renderable or physical primitives,
// which can be interacted with using physics operations like raycasting.
UCLASS(BlueprintType, Category = "Unreal 4D|Component", meta = (BlueprintSpawnableComponent))
class UNREAL4DCORE_API UPrimitive4DComponent : public UScene4DComponent
{
	GENERATED_BODY()
};
