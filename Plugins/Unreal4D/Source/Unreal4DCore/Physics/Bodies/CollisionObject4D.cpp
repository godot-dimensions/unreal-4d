#include "CollisionObject4D.h"

void UCollisionObject4DComponent::RegisterCollisionShape(UShape4DComponent* InShape)
{
	ensure(!_collisionShapes.Contains(InShape));
	_collisionShapes.Add(InShape);
}

void UCollisionObject4DComponent::UnregisterCollisionShape(UShape4DComponent* InShape)
{
	ensure(_collisionShapes.Contains(InShape));
	_collisionShapes.Remove(InShape);
}

const TArray<UShape4DComponent*>& UCollisionObject4DComponent::GetCollisionShapes() const
{
	return _collisionShapes;
}
