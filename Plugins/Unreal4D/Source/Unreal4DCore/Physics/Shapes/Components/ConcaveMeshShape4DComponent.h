#pragma once

#include "Shape4DComponent.h"

#include "../Data/ConcaveMeshShape4DData.h"

#include "ConcaveMeshShape4DComponent.generated.h"

class UTetraMesh4D;

// UConcaveMeshShape4DComponent defines a shape using a tetrahedral mesh.
// It is highly configurable, and can be used to create any arbitrary polytopes,
// even shapes that do not enclose a volume. Unlike most other shapes,
// UConcaveMeshShape4DComponent is not a solid shape, it is hollow.
// It is not an implicit surface, and does not have a well-defined inside or outside.
// UConcaveMeshShape4DComponent is the slowest of the 4D shapes, but it is also the most flexible.
// UConcaveMeshShape4DComponent cannot collide with other UConcaveMeshShape4DComponent shapes,
// and therefore should not be used for moving objects, only static objects.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UConcaveMeshShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	TObjectPtr<UTetraMesh4D> Mesh;

	void SetMesh(UTetraMesh4D* InMesh, UConcaveMeshShape4DData* InShapeData = nullptr);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
