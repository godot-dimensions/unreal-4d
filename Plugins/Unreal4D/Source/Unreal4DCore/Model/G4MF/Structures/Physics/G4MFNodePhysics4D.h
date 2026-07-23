#pragma once

#include "G4MFNodePhysicsMotion4D.h"

#include "G4MFNodePhysics4D.generated.h"

// UG4MFNodePhysics4D describes the physics properties of a G4MF node.
// It allows using shapes to define solid colliders or non-solid triggers, and
// allows defining the motion properties of objects, including mass, inertia, and more.
// Physics properties are not needed for loading the mesh geometry of the scene,
// and may be ignored if the file is only used for visual purposes, or if loading
// into an application that does not support physics and only needs the mesh geometry.
UCLASS(Category = "Unreal 4D|Model|G4MF")
class UNREAL4DCORE_API UG4MFNodePhysics4D : public UG4MFItem4D
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UG4MFNodePhysicsMotion4D> Motion;

	UPROPERTY()
	int32 ColliderShapeIndex = -1;

	UPROPERTY()
	int32 TriggerShapeIndex = -1;

	UPROPERTY()
	TArray<int32> TriggerNodeIndices;

	bool IsEmptyOfMotionAndShapes() const;

	static UG4MFNodePhysics4D* FromJson(UG4MFState4D* InG4MFState, const FJsonObject* InJson);
	TSharedPtr<FJsonObject> ToJson() const;
};
