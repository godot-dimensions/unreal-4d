#pragma once

#include "PhysicsEngine4D.h"

#include "../../Math/Rect4D.h"
#include "../Bodies/DynamicBody4D.h"

#include "AxisAlignedBoxPhysicsEngine4D.generated.h"

// Axis-aligned bounding box physics engine with continuous collision detection using FRect4D.
// This physics engine does not support rotation or angular velocity.
// Non-BoxShape4D shapes will work, but will be converted to boxes.
// Doesn't use any space partitioning, but could be added in the future.
UCLASS()
class UNREAL4DCORE_API UAxisAlignedBoxPhysicsEngine4D : public UPhysicsEngine4D
{
	GENERATED_BODY()

	TMap<UCollisionObject4DComponent*, TArray<FRect4D>> _areaAndBodyRects;
	TMap<UDynamicBody4DComponent*, TArray<FRect4D>> _dynamicBodyRectQueue;
	double _mostRecentPhysicsDeltaTime = 0.0;

	static TArray<FRect4D> CalculateShapeRects(const TArray<UShape4DComponent*>& InShapes);
	static bool DoMovingShapesOverlap(const TArray<FRect4D>& InMovingShapeRects, const TArray<UShape4DComponent*>& InMovingShapes, const TArray<FRect4D>& InObstacleShapeRects, const TArray<UShape4DComponent*>& InObstacleShapes, const FVector4& InMotion, const bool bMovingShapesAreDetector);
	static bool DoStaticShapesOverlap(const TArray<FRect4D>& InShapeRectsA, const TArray<UShape4DComponent*>& InShapesA, const TArray<FRect4D>& InShapeRectsB, const TArray<UShape4DComponent*>& InShapesB, const bool bShapesAAreDetector);

	const TArray<FRect4D>& GetBodyShapeRects(UCollisionObject4DComponent* InBody, const TArray<UShape4DComponent*>& InBodyShapes);
	FKinematicCollision4D CheckMotionUntilObstacle(UCollisionObject4DComponent* InMovingBody, const TArray<FRect4D>& InMovingRects, const FVector4& InMotion, const double InDeltaTime);
	TSet<UArea4DComponent*> StepDynamicRigidBody(UDynamicBody4DComponent* InMovingBody, const double InDeltaTime);

public:
	virtual FKinematicCollision4D MoveAndCollide(UPhysicsBody4DComponent* InBody, const FVector4 &InMotion, const bool InTestOnly, const double InDeltaTime = -1.0) override;
	virtual void MoveArea(UArea4DComponent* InArea, const FVector4 &InMotion) override;
	virtual void PhysicsProcess(const double InDeltaTime) override;
};
