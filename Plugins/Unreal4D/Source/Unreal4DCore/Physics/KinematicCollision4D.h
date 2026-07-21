#pragma once

#include "../Unreal4DDefines.h"

#include "KinematicCollision4D.generated.h"

class UPhysicsBody4DComponent;
class UShape4DComponent;

USTRUCT(BlueprintType, Category = "Unreal 4D|Physics")
struct UNREAL4DCORE_API FKinematicCollision4D
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Unreal 4D|Physics")
	TObjectPtr<UShape4DComponent> MovingShapeComponent = nullptr;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Unreal 4D|Physics")
	TObjectPtr<UShape4DComponent> ObstacleShapeComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics")
	FVector4 Normal = FVector4();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics")
	FVector4 RelativeVelocity = FVector4();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics")
	double TravelRatio = 1.0; // The default represents no collision, so 100% of the motion was completed.

	double GetBounceRatio() const;
	UPhysicsBody4DComponent* GetMovingBodyComponent() const;
	UPhysicsBody4DComponent* GetObstacleBodyComponent() const;
};
