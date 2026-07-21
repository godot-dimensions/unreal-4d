#pragma once

#include "PhysicsBody4D.h"

#include "CharacterBody4D.generated.h"

// Body component class that can move in scripted ways, used primarily for characters or moving platforms.
// The player controller should extend from this class and use it as the actor's root component.
UCLASS(Category = "Unreal 4D|Physics|Body")
class UNREAL4DCORE_API UCharacterBody4DComponent : public UPhysicsBody4DComponent
{
	GENERATED_BODY()

	bool _isOnCeiling = false;
	bool _isOnFloor = false;
	bool _isOnWall = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	double FloorMaxAngle = UNREAL4D_TAU / 4.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	FVector4 LinearVelocity = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	FVector4 UpDirection = FVector4(0.0, 0.0, 1.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Body")
	bool UseBounce = true;

	bool IsOnCeiling() const { return _isOnCeiling; }
	bool IsOnFloor() const { return _isOnFloor; }
	bool IsOnWall() const { return _isOnWall; }

	TArray<FKinematicCollision4D> MoveAndSlide(const double InDeltaTime = -1.0);
};
