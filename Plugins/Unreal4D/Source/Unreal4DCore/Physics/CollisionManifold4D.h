#pragma once

#include "../Unreal4DDefines.h"

#include "CollisionManifold4D.generated.h"

class UPrimitive4DComponent;

// Result of a 4D point query against a collider.
USTRUCT(BlueprintType, Category = "Unreal 4D|Physics")
struct UNREAL4DCORE_API FCollisionManifold4D
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Collision")
	TObjectPtr<UPrimitive4DComponent> ColliderPrimitive = nullptr;

	// Nearest surface point and outward direction in world 4D coordinates.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Collision")
	FVector4 Point = FVector4::Zero();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Collision")
	FVector4 Normal = FVector4::Zero();

	// Signed surface distance in the collider's local coordinates; negative inside.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Collision")
	float Penetration = 0.0f;

	bool IsValid() const;
};
