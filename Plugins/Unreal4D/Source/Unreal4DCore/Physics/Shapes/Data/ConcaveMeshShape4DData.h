#pragma once

#include "Shape4DData.h"

#include "ConcaveMeshShape4DData.generated.h"

// UConcaveMeshShape4DData defines a shape using a set of tetrahedral cells.
// It is highly configurable, and can be used to create any arbitrary polytopes,
// even shapes that do not enclose a volume. Unlike most other shapes,
// UConcaveMeshShape4DData is not a solid shape, it is hollow.
// It is not an implicit surface, and does not have a well-defined inside or outside.
// UConcaveMeshShape4DData is the slowest of the 4D shapes, but it is also the most flexible.
// UConcaveMeshShape4DData cannot collide with other UConcaveMeshShape4DData shapes,
// and therefore should not be used for moving objects, only static objects.
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UConcaveMeshShape4DData : public UShape4DData
{
	GENERATED_BODY()

	TArray<FVector4> _simplexCells;
	TArray<FVector4> _normals;
	TArray<double> _inverseMetricCache;

	static TArray<FVector4> CalculateNormals(const TArray<FVector4>& InSimplexCells);
	static TArray<double> CalculateInverseMetricCache(const TArray<FVector4>& InSimplexCells);

public:
	TArray<FVector4> GetSimplexCells() const { return _simplexCells; }
	void SetSimplexCells(const TArray<FVector4>& InSimplexCells);

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayFrom, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	double GetSignedDistanceToMesh(const FVector4& InLocalPoint, FVector4* OutNearestPointOnTet, int* OutTetrahedronIndex) const;
	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	// Concave shapes do not have any points by definition, they are not solid and not necessarily manifold.
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override { return false; }

	virtual UTetraMesh4D* ToTetraMesh() const override;
	void SetFromMesh(UTetraMesh4D* InMesh4D);
	static UConcaveMeshShape4DData* CreateFromMesh(UTetraMesh4D* InMesh4D);
};
