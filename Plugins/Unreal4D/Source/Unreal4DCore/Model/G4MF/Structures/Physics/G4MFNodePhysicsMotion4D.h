#pragma once

#include "../G4MFItem4D.h"

#include "G4MFNodePhysicsMotion4D.generated.h"

// UG4MFNodePhysicsMotion4D describes the physics motion properties of a G4MF node.
// The existence of this object indicates that the transform of the node is driven by the
// physics engine with the given physical motion properties, such as mass, inertia, and velocity.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFNodePhysicsMotion4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector4 LinearVelocity = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY()
	FString MotionType = "dynamic";

	UPROPERTY()
	double Mass = 1.0;

	UPROPERTY()
	double GravityFactor = 1.0;

	static UG4MFNodePhysicsMotion4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
