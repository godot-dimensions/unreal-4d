#include "KinematicCollision4D.h"

#include "Bodies/PhysicsBody4D.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsSettingsCore.h"
#include "Shapes/Components/Shape4DComponent.h"

double FKinematicCollision4D::GetBounceRatio() const
{
	const uint8 defaultCombineModeUint = UPhysicsSettingsCore::Get()->RestitutionCombineMode.GetIntValue();
	double movingBounce = 0.0;
	double obstacleBounce = 0.0;
	uint8 movingCombineModeUint = defaultCombineModeUint;
	uint8 obstacleCombineModeUint = defaultCombineModeUint;
	if (MovingShapeComponent != nullptr)
	{
		const UPhysicalMaterial* movingMaterial = MovingShapeComponent->PhysicalMaterial4D;
		if (movingMaterial != nullptr)
		{
			movingBounce = movingMaterial->Restitution;
			if (movingMaterial->bOverrideRestitutionCombineMode)
			{
				movingCombineModeUint = movingMaterial->RestitutionCombineMode.GetIntValue();
			}
		}
	}
	if (ObstacleShapeComponent != nullptr)
	{
		const UPhysicalMaterial* obstacleMaterial = ObstacleShapeComponent->PhysicalMaterial4D;
		if (obstacleMaterial != nullptr)
		{
			obstacleBounce = obstacleMaterial->Restitution;
			if (obstacleMaterial->bOverrideRestitutionCombineMode)
			{
				obstacleCombineModeUint = obstacleMaterial->RestitutionCombineMode.GetIntValue();
			}
		}
	}
	const uint8 combineModeUint = FMath::Max(movingCombineModeUint, obstacleCombineModeUint);
	TEnumAsByte<EFrictionCombineMode::Type> combineMode = TEnumAsByte<EFrictionCombineMode::Type>(combineModeUint);
	switch (combineMode)
	{
		case EFrictionCombineMode::Average:
		{
			return (movingBounce + obstacleBounce) * 0.5;
		} break;
		case EFrictionCombineMode::Min:
		{
			return FMath::Min(movingBounce, obstacleBounce);
		} break;
		case EFrictionCombineMode::Multiply:
		{
			return movingBounce * obstacleBounce;
		} break;
		case EFrictionCombineMode::Max:
		{
			return FMath::Max(movingBounce, obstacleBounce);
		} break;
	}
	ensure(false);
	return 0.0;
}

UPhysicsBody4DComponent* FKinematicCollision4D::GetMovingBodyComponent() const
{
	if (MovingShapeComponent == nullptr)
	{
		return nullptr;
	}
	UCollisionObject4DComponent* colObj = MovingShapeComponent->GetAncestorCollisionObject();
	return Cast<UPhysicsBody4DComponent>(colObj);
}

UPhysicsBody4DComponent* FKinematicCollision4D::GetObstacleBodyComponent() const
{
	if (ObstacleShapeComponent == nullptr)
	{
		return nullptr;
	}
	UCollisionObject4DComponent* colObj = ObstacleShapeComponent->GetAncestorCollisionObject();
	return Cast<UPhysicsBody4DComponent>(colObj);
}
