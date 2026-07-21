#pragma once

#include "CollisionObject4D.h"
#include "../KinematicCollision4D.h"

class UPhysicsBody4DComponent;

#include "PhysicsBody4D.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPhysicsBodyImpact4DDelegate, UPhysicsBody4DComponent*, PhysicsBodyComponent, const FKinematicCollision4D&, KinematicCollision);

// Abstract base for solid 4D physics bodies whose shapes can impact other solid 4D physics bodies.
UCLASS(Abstract, Category = "Unreal 4D|Physics|Body")
class UNREAL4DCORE_API UPhysicsBody4DComponent : public UCollisionObject4DComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void OnUnregister() override;

public:
	UPROPERTY()
	double LastPhysicsDeltaTime = 0.0;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics")
	FPhysicsBodyImpact4DDelegate OnBodyImpactedSelf;

	UPROPERTY(BlueprintAssignable, Category = "Unreal 4D|Physics")
	FPhysicsBodyImpact4DDelegate OnSelfImpactedBody;

	FVector4 GetGravity() const;
	FKinematicCollision4D MoveAndCollide(const FVector4& InMotion, const bool InTestOnly = false, const double InDeltaTime = -1.0);
};
