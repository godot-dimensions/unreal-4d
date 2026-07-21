#pragma once

#include "PhysicsBody4D.h"

#include "StaticBody4D.generated.h"

// Body component class that cannot move, used primarily for static objects in the world, such as walls or floors.
UCLASS(Category = "Unreal 4D|Physics|Body")
class UNREAL4DCORE_API UStaticBody4DComponent : public UPhysicsBody4DComponent
{
	GENERATED_BODY()

public:
};
