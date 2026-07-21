#pragma once

#include "../Bodies/Area4D.h"
#include "../Bodies/PhysicsBody4D.h"
#include "../KinematicCollision4D.h"

#include "PhysicsEngine4D.generated.h"

UCLASS()
class UNREAL4DCORE_API UPhysicsEngine4D : public UObject
{
	GENERATED_BODY()

public:
	virtual FKinematicCollision4D MoveAndCollide(UPhysicsBody4DComponent* InBody, const FVector4& InMotion, const bool InTestOnly, const double InDeltaTime) PURE_VIRTUAL(UPhysicsEngine4D::MoveAndCollide, return FKinematicCollision4D(););
	virtual void MoveArea(UArea4DComponent* InArea, const FVector4& InMotion) PURE_VIRTUAL(UPhysicsEngine4D::MoveArea, );
	virtual void PhysicsProcess(const double InDelta) PURE_VIRTUAL(UPhysicsEngine4D::PhysicsProcess, );
};
