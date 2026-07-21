#pragma once

#include "../../../Unreal4DDefines.h"

#include "../../../Math/RaycastResult4D.h"
#include "../../../Math/Rect4D.h"
#include "../../../Math/Transform4D.h"
#include "../../../Model/Mesh/Tetra/TetraMesh4D.h"

#include "Shape4DData.generated.h"

// UShape4DData is the base class for all 4D physics shape data resources, such as
// UBoxShape4DData, USphereShape4DData, UCapsuleShape4DData, UCylinderShape4DData,
// and more. UShape4DData cannot be used on its own, only derived classes may be used.
//
// UShape4DData-derived classes usually describe implicit surfaces, that is,
// shapes with a surface defined by a mathematical function with a well-defined inside and outside,
// but not always, such as a concave mesh shape, which is not an implicit surface.
UCLASS(Abstract, DefaultToInstanced, EditInlineNew, Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UShape4DData : public UObject
{
	GENERATED_BODY()

public:
	virtual double GetHypervolume() const PURE_VIRTUAL(UShape4DData::GetHypervolume, return 0.0;);
	virtual double GetSurfaceVolume() const PURE_VIRTUAL(UShape4DData::GetSurfaceVolume, return 0.0;);

	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InToTarget) const;
	// NOTE: The default max distance is 1e300 instead of infinity because the Unreal Header Tool does not like non-literal default values for parameters.
	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) const;

	virtual double GetSignedDistanceToSurface(const FVector4& InLocalPoint, FVector4* OutNearestPointOnSurface = nullptr) const PURE_VIRTUAL(UShape4DData::GetSignedDistanceToSurface, return 0.0;);

	virtual FVector4 GetNearestPointToLocal(const FVector4& InLocalPoint) const PURE_VIRTUAL(UShape4DData::GetNearestPoint, return FVector4(0.0, 0.0, 0.0, 0.0););
	virtual FVector4 GetSupportPointInLocalDirection(const FVector4& InLocalDirection) const PURE_VIRTUAL(UShape4DData::GetSupportPoint, return FVector4(0.0, 0.0, 0.0, 0.0););
	virtual bool HasLocalPoint(const FVector4& InLocalPoint) const PURE_VIRTUAL(UShape4DData::HasPoint, return false;);

	virtual bool IsEqualExact(const UShape4DData* InShape) const PURE_VIRTUAL(UShape4DData::IsEqualExact, return false;);

	virtual UTetraMesh4D* ToTetraMesh() const PURE_VIRTUAL(UShape4DData::ToTetraMesh, return nullptr;);
};
