#pragma once

#include "Shape4DData.h"

#include "ConvexHullShape4DData.generated.h"

// UConvexHullShape4DData is a solid 4D convex shape that is defined by a set of points
// in 4D space. The convex hull is the smallest convex shape that contains all the points.
// This shape can be used to create bounding shapes for arbitrary 4D meshes or point clouds.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UConvexHullShape4DData : public UShape4DData {
	GENERATED_BODY()

	TArray<FVector4> _points;

public:
	TArray<FVector4> GetPoints() const { return _points; }
	void SetPoints(const TArray<FVector4>& InPoints);

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;

	virtual UTetraMesh4D* ToTetraMesh() const override;
	static UConvexHullShape4DData* CreateFromMesh(UMesh4D* InMesh);
};
