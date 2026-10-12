#include "CollisionManifold4D.h"

#include "../Scene/Components/Primitive4DComponent.h"

bool FCollisionManifold4D::IsValid() const
{
	return ::IsValid(ColliderPrimitive.Get());
}
