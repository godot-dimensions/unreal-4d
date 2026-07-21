#pragma once

#include "../../Scene/Components/Scene4DComponent.h"
#include "../Shapes/Components/Shape4DComponent.h"

#include "CollisionObject4D.generated.h"

class UArea4DComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArea4DComponentDelegate, UArea4DComponent*, AreaComponent);

// Abstract base of all 4D objects that can collide with 4D shapes, either solid or non-solid overlaps.
UCLASS(Abstract, Category = "Unreal 4D|Physics")
class UNREAL4DCORE_API UCollisionObject4DComponent : public UScene4DComponent
{
	GENERATED_BODY()

	TArray<UShape4DComponent*> _collisionShapes;

public:
	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics")
	FArea4DComponentDelegate OnSelfEnteredArea;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics")
	FArea4DComponentDelegate OnSelfExitedArea;

	void RegisterCollisionShape(UShape4DComponent* InShape);
	void UnregisterCollisionShape(UShape4DComponent* InShape);

	const TArray<UShape4DComponent*>& GetCollisionShapes() const;
};
