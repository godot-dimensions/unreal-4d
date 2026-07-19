#pragma once

#include "../Unreal4DDefines.h"

#include "RaycastResult4D.generated.h"

// This is located in the 4D math folder because it is used by FRect4D.
USTRUCT(BlueprintType, Category = "Unreal 4D|Math")
struct UNREAL4DCORE_API FRaycastResult4D
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The point of intersection where the ray hit the shape or mesh.
	FVector4 Point = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The normal vector at the point of intersection on the shape or mesh, perpendicular to the surface.
	FVector4 Normal = FVector4(0.0, 0.0, 0.0, 0.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The physics body component that was hit by the raycast (an ancestor of the shape or mesh that was hit).
	// If the raycast hit a shape that is not a part of a physics body, this will be nullptr.
	TWeakObjectPtr<UObject> BodyComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The physics shape or mesh component that includes the shape which was hit by the raycast.
	// If the raycast did not hit a physics shape component or mesh component, this will be nullptr.
	TWeakObjectPtr<UObject> ShapeComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The distance from the ray's origin to the point of intersection.
	double Distance = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// The index of the tetrahedral cell the ray hit. Only set for TetraMesh4D-based shapes.
	int32 CellIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Math|Raycast Result")
	// True if the raycast hit something, false if it did not hit anything.
	bool bHit = false;

	// Returns a FRaycastResult4D with bHit set to true, and all other values empty/zero/default.
	static FRaycastResult4D EmptyWithHit()
	{
		FRaycastResult4D result;
		result.bHit = true;
		return result;
	}
};
