#pragma once

#include "Shape4DComponent.h"

#include "OrthoplexShape4DComponent.generated.h"

// UOrthoplexShape4D describes a 4D orthoplex physics shape component using a Size as an FVector4.
// The orthoplex is centered at the origin, and is axis-aligned by default; if you want an orthoplex
// facing a different direction, you can rotate the component's 4D transform.
//
// An orthoplex is a 4D cross-polytope with 8 vertices, plus or minus on
// each axis (-X, +X, -Y, +Y, -Z, +Z, -W, +W), and 16 tetrahedral cells.
// The 3D version of an orthoplex is the octahedron, which has 6 vertices (-X, +X, -Y, +Y, -Z, +Z)
// and 8 triangular faces in each octant. The 2D version is a diamond (square rotated 45 degrees),
// which has 4 vertices (-X, +X, -Y, +Y) and 4 edges in each quadrant. Orthoplexes exist in
// all dimensions, and are the dual of boxes (square/cube/hypercube).
UCLASS(Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UOrthoplexShape4DComponent : public UShape4DComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	FVector4 Size = FVector4(100.0, 100.0, 100.0, 100.0);

	void SetSize(const FVector4& InNewSize);
	FVector4 GetHalfExtents() const;
	void SetHalfExtents(const FVector4& InHalfExtents);

	virtual void UpdateComponentFromShape() override;
	virtual void UpdateShapeFromComponent() override;
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
};
