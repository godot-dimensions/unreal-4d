#include "AxisAlignedBoxPhysicsEngine4D.h"

#include "../Bodies/CharacterBody4D.h"
#include "../Bodies/DynamicBody4D.h"
#include "../KinematicCollision4D.h"
#include "PhysicsServer4D.h"

TArray<FRect4D> UAxisAlignedBoxPhysicsEngine4D::CalculateShapeRects(const TArray<UShape4DComponent*>& InShapes)
{
	TArray<FRect4D> shapeRects;
	const int shapeCount = InShapes.Num();
	shapeRects.SetNum(shapeCount);
	for (int shapeIndex = 0; shapeIndex < shapeCount; shapeIndex++)
	{
		const UShape4DComponent* shapeNode = InShapes[shapeIndex];
		shapeRects[shapeIndex] = shapeNode->GetRectBoundsGlobal4D(FTransform4D::Identity());
	}
	return shapeRects;
}

bool UAxisAlignedBoxPhysicsEngine4D::DoMovingShapesOverlap(const TArray<FRect4D>& InMovingShapeRects, const TArray<UShape4DComponent*>& InMovingShapes, const TArray<FRect4D>& InObstacleShapeRects, const TArray<UShape4DComponent*>& InObstacleShapes, const FVector4& InMotion, const bool bMovingShapesAreDetector)
{
	ensure(InMovingShapeRects.Num() == InMovingShapes.Num());
	ensure(InObstacleShapeRects.Num() == InObstacleShapes.Num());
	const int shapeCountA = InMovingShapeRects.Num();
	const int shapeCountB = InObstacleShapeRects.Num();
	for (int shapeIndexA = 0; shapeIndexA < shapeCountA; shapeIndexA++)
	{
		const FRect4D& shapeRectA = InMovingShapeRects[shapeIndexA];
		for (int shapeIndexB = 0; shapeIndexB < shapeCountB; shapeIndexB++)
		{
			const UShape4DComponent* detectorShape = bMovingShapesAreDetector ? InMovingShapes[shapeIndexA] : InObstacleShapes[shapeIndexB];
			const UShape4DComponent* detectedShape = bMovingShapesAreDetector ? InObstacleShapes[shapeIndexB] : InMovingShapes[shapeIndexA];
			if (detectorShape != nullptr && detectedShape != nullptr)
			{
				const bool bDoLayersOverlap = detectorShape->CollisionMask & detectedShape->CollisionLayer;
				if (!bDoLayersOverlap)
				{
					continue;
				}
			}
			const FRect4D& shapeRectB = InObstacleShapeRects[shapeIndexB];
			if (shapeRectA.ContinuousCollisionOverlaps(InMotion, shapeRectB))
			{
				return true;
			}
		}
	}
	return false;
}

bool UAxisAlignedBoxPhysicsEngine4D::DoStaticShapesOverlap(const TArray<FRect4D>& InShapeRectsA, const TArray<UShape4DComponent*>& InShapesA, const TArray<FRect4D>& InShapeRectsB, const TArray<UShape4DComponent*>& InShapesB, const bool bShapesAAreDetector)
{
	const int shapeCountA = InShapeRectsA.Num();
	const int shapeCountB = InShapeRectsB.Num();
	for (int shapeIndexA = 0; shapeIndexA < shapeCountA; shapeIndexA++)
	{
		const FRect4D& shapeRectA = InShapeRectsA[shapeIndexA];
		for (int shapeIndexB = 0; shapeIndexB < shapeCountB; shapeIndexB++)
		{
			const UShape4DComponent* detectorShape = bShapesAAreDetector ? InShapesA[shapeIndexA] : InShapesB[shapeIndexB];
			const UShape4DComponent* detectedShape = bShapesAAreDetector ? InShapesB[shapeIndexB] : InShapesA[shapeIndexA];
			if (detectorShape != nullptr && detectedShape != nullptr)
			{
				const bool bDoLayersOverlap = detectorShape->CollisionMask & detectedShape->CollisionLayer;
				if (!bDoLayersOverlap)
				{
					continue;
				}
			}
			const FRect4D& shapeRectB = InShapeRectsB[shapeIndexB];
			// Inclusive is required here to pair well with the behavior of the continuous collision function.
			if (shapeRectA.IntersectsInclusive(shapeRectB))
			{
				return true;
			}
		}
	}
	return false;
}

const TArray<FRect4D>& UAxisAlignedBoxPhysicsEngine4D::GetBodyShapeRects(UCollisionObject4DComponent* InMovingBody, const TArray<UShape4DComponent*>& InBodyShapes)
{
	if (!_areaAndBodyRects.Contains(InMovingBody) || _areaAndBodyRects[InMovingBody].Num() != InBodyShapes.Num())
	{
		_areaAndBodyRects.Add(InMovingBody, CalculateShapeRects(InBodyShapes));
	}
	return _areaAndBodyRects[InMovingBody];
}

FKinematicCollision4D UAxisAlignedBoxPhysicsEngine4D::CheckMotionUntilObstacle(UCollisionObject4DComponent* InMovingBody, const TArray<FRect4D>& InMovingRects, const FVector4& InMotion, const double InDeltaTime)
{
	ensure(InMovingBody != nullptr);
	FKinematicCollision4D ret;
	// Get information about bodies.
	const TArray<UShape4DComponent*>& movingShapes = InMovingBody->GetCollisionShapes();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = InMovingBody->GetWorld()->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	const TArray<UPhysicsBody4DComponent*>& obstacleBodyComponents = physicsServer4D->GetPhysicsBodyComponents();
	const int obstacleBodyCount = obstacleBodyComponents.Num();
	// Calculate the ratio of the desired motion that can be taken before hitting an obstacle.
	double travelRatio = 1.0;
	FVector4 outNormal;
	for (int obstacleBodyIndex = 0; obstacleBodyIndex < obstacleBodyCount; obstacleBodyIndex++)
	{
		UPhysicsBody4DComponent* obstacleBodyComponent = Cast<UPhysicsBody4DComponent>(obstacleBodyComponents[obstacleBodyIndex]);
		if (obstacleBodyComponent == nullptr)
		{
			continue;
		}
		if (obstacleBodyComponent == InMovingBody)
		{
			continue; // Don't collide with ourself.
		}
		UCharacterBody4DComponent* obstacleCharacterBody = Cast<UCharacterBody4DComponent>(obstacleBodyComponent);
		UDynamicBody4DComponent* obstacleDynamicBody = Cast<UDynamicBody4DComponent>(obstacleBodyComponent);
		const TArray<UShape4DComponent*>& obstacleBodyShapes = obstacleBodyComponent->GetCollisionShapes();
		const TArray<FRect4D>& obstacleShapeRects = GetBodyShapeRects(obstacleBodyComponent, obstacleBodyShapes);
		// Iterate through both sets of shapes and check for collisions.
		for (int movingShapeIndex = 0; movingShapeIndex < InMovingRects.Num(); movingShapeIndex++)
		{
			UShape4DComponent* movingShape = Cast<UShape4DComponent>(movingShapes[movingShapeIndex]);
			if (movingShape == nullptr)
			{
				continue;
			}
			const FRect4D& movingShapeRect = InMovingRects[movingShapeIndex];
			for (int obstacleShapeIndex = 0; obstacleShapeIndex < obstacleShapeRects.Num(); obstacleShapeIndex++)
			{
				UShape4DComponent* obstacleShape = Cast<UShape4DComponent>(obstacleBodyShapes[obstacleShapeIndex]);
				if (obstacleShape == nullptr)
				{
					continue;
				}
				const bool bDoLayersOverlap = movingShape->CollisionMask & obstacleShape->CollisionLayer;
				if (bDoLayersOverlap)
				{
					const FRect4D& obstacleShapeRect = obstacleShapeRects[obstacleShapeIndex];
					// The CCD function operates with a relative velocity, so we can check for body types and use their velocity.
					FVector4 relativeMotion = InMotion;
					if (obstacleCharacterBody != nullptr)
					{
						relativeMotion -= obstacleCharacterBody->LinearVelocity * InDeltaTime;
					}
					else if (obstacleDynamicBody != nullptr)
					{
						relativeMotion -= obstacleDynamicBody->LinearVelocity * InDeltaTime;
					}
					FVector4 thisNormal;
					const double thisRatio = movingShapeRect.ContinuousCollisionDepth(relativeMotion, obstacleShapeRect, &thisNormal);
					if (thisRatio < travelRatio)
					{
						travelRatio = thisRatio;
						outNormal = thisNormal; // We remember the normal only from the nearest collision.
						ret.MovingShapeComponent = movingShape;
						ret.ObstacleShapeComponent = obstacleShape;
						if (InDeltaTime != 0.0)
						{
							ret.RelativeVelocity = relativeMotion / InDeltaTime;
						}
					}
				}
			}
		}
	}
	ret.Normal = outNormal;
	ret.TravelRatio = travelRatio;
	return ret;
}

TSet<UArea4DComponent*> UAxisAlignedBoxPhysicsEngine4D::StepDynamicRigidBody(UDynamicBody4DComponent* InMovingBody, double InDeltaTime)
{
	const TArray<UShape4DComponent*>& movingShapes = InMovingBody->GetCollisionShapes();
	const FVector4 startGlobalPosition = InMovingBody->GetGlobalPosition4D();
	// Used for determining overlaps with UArea4DComponents.
	UPhysicsServer4DWorldSubsystem* physicsServer4D = InMovingBody->GetWorld()->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	const TArray<UArea4DComponent*>& areaComponents = physicsServer4D->GetAreaComponents();
	const int areaCount = areaComponents.Num();
	TSet<UArea4DComponent*> overlappingAreas;
	// Calculate the motion.
	FVector4 actualMotion = FVector4(0.0, 0.0, 0.0, 0.0);
	{
		FVector4 linearVelocity = InMovingBody->LinearVelocity;
		FVector4 desiredMotion = linearVelocity * InDeltaTime;
		// In this case, use direct access to the HashMap instead of the `GetBodyShapeRects()` function,
		// since we know that the rects have just been calculated at the top of `PhysicsProcess()`.
		TArray<FRect4D> currentMovingShapeRects = _areaAndBodyRects[InMovingBody];
		constexpr int MaxDynamicBodyIterations = 10;
		for (int iteration = 0; iteration < MaxDynamicBodyIterations; iteration++)
		{
			FKinematicCollision4D collision = CheckMotionUntilObstacle(InMovingBody, currentMovingShapeRects, desiredMotion, InDeltaTime);
			FVector4 motionThisIteration;
			double travelRatio = collision.TravelRatio;
			if (travelRatio == 1.0)
			{
				// Move the rest of the way.
				motionThisIteration = desiredMotion;
				actualMotion += desiredMotion;
				desiredMotion = FVector4(0.0, 0.0, 0.0, 0.0);
			}
			else
			{
				// Move part of the way, since we hit an obstacle.
				// The safeMargin tries to prevent objects from getting too close, but is not enough by itself.
				// We still need a proper depenetration algorithm to fix already-overlapping objects.
				const FVector4 safeMargin = collision.Normal * PHYSICS_4D_SAFE_MARGIN;
				motionThisIteration = desiredMotion * travelRatio + safeMargin;
				actualMotion += motionThisIteration;
				desiredMotion = desiredMotion * (1.0 - travelRatio);
				// Emit signals.
				UPhysicsBody4DComponent* obstacleBody = collision.GetObstacleBodyComponent();
				if (obstacleBody)
				{
					obstacleBody->OnBodyImpactedSelf.Broadcast(InMovingBody, collision);
					InMovingBody->OnSelfImpactedBody.Broadcast(obstacleBody, collision);
				}
				// Bouncing.
				const double bounceRatio = collision.GetBounceRatio();
				desiredMotion = UVectorFuncs4D::Vector4_BounceRatio(desiredMotion, collision.Normal, bounceRatio);
				linearVelocity = UVectorFuncs4D::Vector4_BounceRatio(linearVelocity, collision.Normal, bounceRatio);
			}
			// Check for overlaps with areas.
			for (int areaIndex = 0; areaIndex < areaCount; areaIndex++)
			{
				UArea4DComponent* areaComponent = Cast<UArea4DComponent>(areaComponents[areaIndex]);
				if (overlappingAreas.Contains(areaComponent))
				{
					continue; // Already overlapping, no need to check again.
				}
				const TArray<UShape4DComponent*>& staticShapes = areaComponent->GetCollisionShapes();
				const TArray<FRect4D>& staticRects = GetBodyShapeRects(areaComponent, staticShapes);
				if (DoMovingShapesOverlap(currentMovingShapeRects, movingShapes, staticRects, staticShapes, motionThisIteration, false))
				{
					overlappingAreas.Add(areaComponent);
				}
			}
			// Update the position and current shape rects for next iteration.
			InMovingBody->SetGlobalPosition4D(startGlobalPosition + actualMotion);
			currentMovingShapeRects = CalculateShapeRects(movingShapes);
			if (desiredMotion.SizeSquared() < UNREAL4D_COMPARISON_EPSILON)
			{
				break;
			}
		}
		InMovingBody->LinearVelocity = linearVelocity;
		_dynamicBodyRectQueue.Add(InMovingBody, currentMovingShapeRects);
	}
	return overlappingAreas;
}

FKinematicCollision4D UAxisAlignedBoxPhysicsEngine4D::MoveAndCollide(UPhysicsBody4DComponent* InMovingBody, const FVector4& InMotion, const bool InTestOnly, const double InDeltaTime)
{
	const TArray<UShape4DComponent*>& movingShapes = InMovingBody->GetCollisionShapes();
	const double deltaTime = InDeltaTime < 0.0 ? _mostRecentPhysicsDeltaTime : InDeltaTime;
	// Calculate the motion.
	const TArray<FRect4D> movingRectsBefore = GetBodyShapeRects(InMovingBody, movingShapes);
	FKinematicCollision4D collision = CheckMotionUntilObstacle(InMovingBody, movingRectsBefore, InMotion, deltaTime);
	if (InTestOnly)
	{
		return collision;
	}
	double travelRatio = collision.TravelRatio;
	// A negative travel ratio means we're inside the obstacle.
	// TODO: Implement a proper depenetration algorithm. For now, just stop moving.
	if (travelRatio < 0.0)
	{
		collision.TravelRatio = 0.0;
		travelRatio = 0.0;
	}
	// The safeMargin tries to prevent objects from getting too close, but is not enough by itself.
	// We still need a proper depenetration algorithm to fix already-overlapping objects.
	FVector4 safeMargin = collision.Normal * PHYSICS_4D_SAFE_MARGIN;
	InMovingBody->SetGlobalPosition4D(InMovingBody->GetGlobalPosition4D() + InMotion* travelRatio + safeMargin);
	TArray<FRect4D> movingRectsAfter = CalculateShapeRects(movingShapes);
	// Check for overlaps with areas.
	UPhysicsServer4DWorldSubsystem* physicsServer4D = InMovingBody->GetWorld()->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	const TArray<UArea4DComponent*>& areaComponents = physicsServer4D->GetAreaComponents();
	for (int areaIndex = 0; areaIndex < areaComponents.Num(); areaIndex++)
	{
		UArea4DComponent* areaComponent = Cast<UArea4DComponent>(areaComponents[areaIndex]);
		const TArray<UShape4DComponent*> staticShapes = areaComponent->GetCollisionShapes();
		const TArray<FRect4D>& staticRects = GetBodyShapeRects(areaComponent, staticShapes);
		if (DoMovingShapesOverlap(movingRectsBefore, movingShapes, staticRects, staticShapes, InMotion* travelRatio, false))
		{
			if (!DoStaticShapesOverlap(movingRectsBefore, movingShapes, staticRects, staticShapes, false))
			{
				// If they overlap during the motion, but didn't before, emit the entered signals.
				areaComponent->OnBodyEnteredSelf.Broadcast(InMovingBody);
				InMovingBody->OnSelfEnteredArea.Broadcast(areaComponent);
			}
			if (!DoStaticShapesOverlap(movingRectsAfter, movingShapes, staticRects, staticShapes, false))
			{
				// If they overlap during the motion, but don't after, emit the exited signals.
				areaComponent->OnBodyExitedSelf.Broadcast(InMovingBody);
				InMovingBody->OnSelfExitedArea.Broadcast(areaComponent);
			}
		}
	}
	_areaAndBodyRects[InMovingBody] = movingRectsAfter;
	return collision;
}

void UAxisAlignedBoxPhysicsEngine4D::MoveArea(UArea4DComponent* InMovingArea, const FVector4& InMotion)
{
	const TArray<UShape4DComponent*>& movingShapes = InMovingArea->GetCollisionShapes();
	const TArray<FRect4D> movingRectsBefore = GetBodyShapeRects(InMovingArea, movingShapes);
	// Area nodes are not stopped by anything, so we can just move them and check for overlaps after.
	InMovingArea->SetGlobalPosition4D(InMovingArea->GetGlobalPosition4D() + InMotion);
	TArray<FRect4D> movingRectsAfter = CalculateShapeRects(movingShapes);
	// Check for overlaps with bodies and other areas.
	UPhysicsServer4DWorldSubsystem* physicsServer4D = InMovingArea->GetWorld()->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	const TArray<UArea4DComponent*>& areaComponents = physicsServer4D->GetAreaComponents();
	for (int areaIndex = 0; areaIndex < areaComponents.Num(); areaIndex++)
	{
		UArea4DComponent* staticAreaComponent = Cast<UArea4DComponent>(areaComponents[areaIndex]);
		if (staticAreaComponent == InMovingArea)
		{
			continue; // Don't collide with ourself.
		}
		const TArray<UShape4DComponent*> staticShapes = staticAreaComponent->GetCollisionShapes();
		const TArray<FRect4D>& staticRects = GetBodyShapeRects(staticAreaComponent, staticShapes);
		if (DoMovingShapesOverlap(movingRectsBefore, movingShapes, staticRects, staticShapes, InMotion, false))
		{
			if (!DoStaticShapesOverlap(movingRectsBefore, movingShapes, staticRects, staticShapes, false))
			{
				// If they overlap during the motion, but didn't before, emit the entered signals.
				staticAreaComponent->OnAreaEnteredSelf.Broadcast(InMovingArea);
				InMovingArea->OnSelfEnteredArea.Broadcast(staticAreaComponent);
			}
			if (!DoStaticShapesOverlap(movingRectsAfter, movingShapes, staticRects, staticShapes, false))
			{
				// If they overlap during the motion, but don't after, emit the exited signals.
				staticAreaComponent->OnAreaExitedSelf.Broadcast(InMovingArea);
				InMovingArea->OnSelfExitedArea.Broadcast(staticAreaComponent);
			}
		}
	}
	const TArray<UPhysicsBody4DComponent*>& bodyComponents = physicsServer4D->GetPhysicsBodyComponents();
	for (int bodyIndex = 0; bodyIndex < bodyComponents.Num(); bodyIndex++)
	{
		UPhysicsBody4DComponent* staticBodyNode = Cast<UPhysicsBody4DComponent>(bodyComponents[bodyIndex]);
		const TArray<UShape4DComponent*> staticBodyShapes = staticBodyNode->GetCollisionShapes();
		const TArray<FRect4D>& staticBodyRects = GetBodyShapeRects(staticBodyNode, staticBodyShapes);
		if (DoMovingShapesOverlap(movingRectsBefore, movingShapes, staticBodyRects, staticBodyShapes, InMotion, true))
		{
			if (!DoStaticShapesOverlap(movingRectsBefore, movingShapes, staticBodyRects, staticBodyShapes, true))
			{
				// If they overlap during the motion, but didn't before, emit the entered signals.
				staticBodyNode->OnSelfEnteredArea.Broadcast(InMovingArea);
				InMovingArea->OnBodyEnteredSelf.Broadcast(staticBodyNode);
			}
			if (!DoStaticShapesOverlap(movingRectsAfter, movingShapes, staticBodyRects, staticBodyShapes, true))
			{
				// If they overlap during the motion, but don't after, emit the exited signals.
				staticBodyNode->OnSelfExitedArea.Broadcast(InMovingArea);
				InMovingArea->OnBodyExitedSelf.Broadcast(staticBodyNode);
			}
		}
	}
	_areaAndBodyRects[InMovingArea] = movingRectsAfter;
}

void UAxisAlignedBoxPhysicsEngine4D::PhysicsProcess(const double InDeltaTime)
{
	_mostRecentPhysicsDeltaTime = InDeltaTime;
	// Recalculate rects for all areas and bodies each frame, just in case they changed.
	_areaAndBodyRects.Empty();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = GetWorld()->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	const TArray<UArea4DComponent*>& areaComponents = physicsServer4D->GetAreaComponents();
	const TArray<UPhysicsBody4DComponent*>& bodyComponents = physicsServer4D->GetPhysicsBodyComponents();
	for (int areaIndex = 0; areaIndex < areaComponents.Num(); areaIndex++)
	{
		UCollisionObject4DComponent* areaComponent = Cast<UCollisionObject4DComponent>(areaComponents[areaIndex]);
		_areaAndBodyRects.Add(areaComponent, CalculateShapeRects(areaComponent->GetCollisionShapes()));
	}
	for (int bodyIndex = 0; bodyIndex < bodyComponents.Num(); bodyIndex++)
	{
		UCollisionObject4DComponent* bodyNode = Cast<UCollisionObject4DComponent>(bodyComponents[bodyIndex]);
		_areaAndBodyRects.Add(bodyNode, CalculateShapeRects(bodyNode->GetCollisionShapes()));
		// Also since we're in a loop with bodies, apply gravity here, before stepping.
		UDynamicBody4DComponent* dynamicBody = Cast<UDynamicBody4DComponent>(bodyNode);
		if (dynamicBody != nullptr)
		{
			dynamicBody->ApplyAcceleration(dynamicBody->GetScaledGravity());
		}
	}
	// Step dynamic rigid bodies with the calculated rects.
	for (int bodyIndex = 0; bodyIndex < bodyComponents.Num(); bodyIndex++)
	{
		UDynamicBody4DComponent* dynamicBody = Cast<UDynamicBody4DComponent>(bodyComponents[bodyIndex]);
		if (dynamicBody != nullptr)
		{
			const TArray<UShape4DComponent*>& movingShapes = dynamicBody->GetCollisionShapes();
			TSet<UArea4DComponent*> overlappingAreas = StepDynamicRigidBody(dynamicBody, InDeltaTime);
			// Check for overlaps with areas. The TSet tells us which areas overlap
			// via the whole CCD motion, but we need to check before and after.
			for (UArea4DComponent* areaComponent : overlappingAreas)
			{
				const TArray<UShape4DComponent*>& staticShapes = areaComponent->GetCollisionShapes();
				// In this case, use direct access to the HashMap instead of the `GetBodyShapeRects()` function,
				// since we know that the rects have just been calculated at the top of this function.
				const TArray<FRect4D>& staticRects = _areaAndBodyRects[areaComponent];
				if (!DoStaticShapesOverlap(_areaAndBodyRects[dynamicBody], movingShapes, staticRects, staticShapes, false))
				{
					// If they overlap during the motion, but didn't before, emit the entered signals.
					areaComponent->OnBodyEnteredSelf.Broadcast(dynamicBody);
					dynamicBody->OnSelfEnteredArea.Broadcast(areaComponent);
				}
				if (!DoStaticShapesOverlap(_dynamicBodyRectQueue[dynamicBody], movingShapes, staticRects, staticShapes, false))
				{
					// If they overlap during the motion, but don't after, emit the exited signals.
					areaComponent->OnBodyExitedSelf.Broadcast(dynamicBody);
					dynamicBody->OnSelfExitedArea.Broadcast(areaComponent);
				}
			}
		}
	}
	// Update rects for rigid bodies from queue. Doing this separately from stepping
	// allows rigid bodies to move next to each other and hit each other, with the
	// same behavior regardless of the order they are processed in.
	for (const TPair<UDynamicBody4DComponent*, TArray<FRect4D>>& item : _dynamicBodyRectQueue)
	{
		_areaAndBodyRects[item.Key] = item.Value;
	}
	_dynamicBodyRectQueue.Empty();
}
