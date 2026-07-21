#pragma once

#include "Shape4DComponent.h"

#include "../Data/ConvexHullShape4DData.h"

#include "ConvexHullShape4DComponent.generated.h"

class UMesh4D;

// UConvexHullShape4DComponent is a solid 4D convex shape that is defined by a mesh
// in 4D space. The convex hull is calculated from the mesh vertices.
// This shape can be used to create bounding shapes for arbitrary 4D meshes or point clouds.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UConvexHullShape4DComponent : public UShape4DComponent {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	TObjectPtr<UMesh4D> Mesh;

	void SetMesh(UMesh4D* InMesh, UConvexHullShape4DData* InShapeData = nullptr);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
