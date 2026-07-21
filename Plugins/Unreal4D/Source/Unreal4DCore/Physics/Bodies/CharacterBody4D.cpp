#include "CharacterBody4D.h"

#include "../KinematicCollision4D.h"

constexpr int PHYSICS_4D_MAX_MOVE_AND_SLIDE_ITERATIONS = 10;

TArray<FKinematicCollision4D> UCharacterBody4DComponent::MoveAndSlide(const double InDeltaTime)
{
	TArray<FKinematicCollision4D> collisions;
	const double deltaTime = InDeltaTime < 0.0 ? LastPhysicsDeltaTime : InDeltaTime;
	_isOnCeiling = false;
	_isOnFloor = false;
	_isOnWall = false;
	{
		FVector4 desiredMotion = LinearVelocity * deltaTime;
		for (int iteration = 0; iteration < PHYSICS_4D_MAX_MOVE_AND_SLIDE_ITERATIONS; iteration++)
		{
			FKinematicCollision4D collision = MoveAndCollide(desiredMotion, false, deltaTime);
			const double travelRatio = collision.TravelRatio;
			if (travelRatio == 1.0)
			{
				// This last MoveAndCollide call moved us the rest of the way, with no collisions.
				break;
			}
			// This last MoveAndCollide call moved us part of the way. We need to deal with the collision.
			const FVector4 normal = collision.Normal;
			desiredMotion = desiredMotion * (1.0 - travelRatio);
			// Is on floor/ceiling/wall checks.
			if (UpDirection == FVector4(0.0, 0.0, 0.0, 0.0))
			{
				_isOnWall = true;
			}
			else
			{
				const double angle = UVectorFuncs4D::Vector4_AngleTo(UpDirection, normal);
				if (angle < FloorMaxAngle)
				{
					_isOnFloor = true;
				}
				else if (angle > UE_DOUBLE_PI - FloorMaxAngle)
				{
					_isOnCeiling = true;
				}
				else
				{
					_isOnWall = true;
				}
			}
			// Bouncing.
			if (UseBounce)
			{
				const double bounceRatio = collision.GetBounceRatio();
				desiredMotion = UVectorFuncs4D::Vector4_BounceRatio(desiredMotion, collision.Normal, bounceRatio);
				LinearVelocity = UVectorFuncs4D::Vector4_BounceRatio(LinearVelocity, collision.Normal, bounceRatio);
			}
			else
			{
				desiredMotion = UVectorFuncs4D::Vector4_Slide(desiredMotion, collision.Normal);
				LinearVelocity = UVectorFuncs4D::Vector4_Slide(LinearVelocity, collision.Normal);
			}
			// Final checks.
			collisions.Add(collision);
			if (desiredMotion.SizeSquared() < UNREAL4D_COMPARISON_EPSILON)
			{
				break;
			}
		}
	}
	return collisions;
}
