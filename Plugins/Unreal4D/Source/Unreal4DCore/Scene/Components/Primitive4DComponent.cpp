#include "Primitive4DComponent.h"

bool UPrimitive4DComponent::QueryPointCollision(const FVector4& InPoint, FCollisionManifold4D& OutCollision) const
{
	return false;
}

// Raycasting.

FRaycastResult4D UPrimitive4DComponent::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero)
{
	// This function should be overridden by derived classes that have a shape or mesh to raycast against.
	// Fallback implementation that uses GetRectBoundsLocal4D to determine if the ray intersects the bounding box of the node.
	const FRect4D localBounds = GetRectBoundsLocal4D(FTransform4D::Identity());
	return localBounds.RaycastIntersectsFull(InLocalRayOrigin, InLocalRayDirection, InMaxDistance, bInInsideIsZero);
}
