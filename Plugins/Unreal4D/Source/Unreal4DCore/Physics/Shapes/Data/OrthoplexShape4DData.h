#pragma once

#include "Shape4DData.h"

#include "OrthoplexShape4DData.generated.h"

// UOrthoplexShape4D describes a 4D orthoplex physics shape data using a Size as an FVector4.
// The orthoplex is centered at the origin, and is axis-aligned by default; if you want an orthoplex
// facing a different direction, you can rotate the component the orthoplex is attached to.
//
// An orthoplex is a 4D cross-polytope with 8 vertices, plus or minus on
// each axis (-X, +X, -Y, +Y, -Z, +Z, -W, +W), and 16 tetrahedral cells.
// The 3D version of an orthoplex is the octahedron, which has 6 vertices (-X, +X, -Y, +Y, -Z, +Z)
// and 8 triangular faces in each octant. The 2D version is a diamond (square rotated 45 degrees),
// which has 4 vertices (-X, +X, -Y, +Y) and 4 edges in each quadrant. Orthoplexes exist in
// all dimensions, and are the dual of boxes (square/cube/hypercube).
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UOrthoplexShape4DData : public UShape4DData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

	FVector4 GetHalfExtents() const;
	void SetHalfExtents(const FVector4& InHalfExtents);

	virtual double GetHypervolume() const override;
	virtual double GetSurfaceVolume() const override;
	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InTransformToTarget = FTransform4D()) const override;

	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const override;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const override;

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const override;
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const override;
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const override;

	virtual bool IsEqualExact(const UShape4DData* InShape) const override;

	virtual UTetraMesh4D* ToTetraMesh() const override;
};
