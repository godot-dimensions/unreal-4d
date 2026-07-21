#pragma once

#include "CollisionObject4D.h"

#include "Area4D.generated.h"

class UPhysicsBody4DComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhysicsBody4DComponentDelegate, UPhysicsBody4DComponent*, PhysicsBodyComponent);

// Collision component class that represents non-solid volumes in the world, such as triggers, sensors, detectors, etc.
UCLASS(Category = "Unreal 4D|Physics|Area")
class UNREAL4DCORE_API UArea4DComponent : public UCollisionObject4DComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics|Area")
	FArea4DComponentDelegate OnAreaEnteredSelf;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics|Area")
	FArea4DComponentDelegate OnAreaExitedSelf;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics|Area")
	FPhysicsBody4DComponentDelegate OnBodyEnteredSelf;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics|Area")
	FPhysicsBody4DComponentDelegate OnBodyExitedSelf;

	void MoveArea(const FVector4& InMotion);
};
