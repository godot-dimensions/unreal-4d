#pragma once

#include "Scene4DComponent.h"

#include "../../Physics/Shapes/Data/Shape4DData.h"

#include "Primitive4DComponent.generated.h"

// Base class for 4D components that represent renderable or physical primitives,
// which can be interacted with using physics operations like raycasting.
UCLASS(BlueprintType, Category = "Unreal 4D|Component", meta = (BlueprintSpawnableComponent))
class UNREAL4DCORE_API UPrimitive4DComponent : public UScene4DComponent
{
	GENERATED_BODY()

public:
	virtual UShape4DData* GetShapeData() const { return nullptr; }

	// Raycasting.
	// NOTE: The default max distance is 1e300 instead of infinity because the Unreal Header Tool does not like non-literal default values for parameters.
	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false);
};
