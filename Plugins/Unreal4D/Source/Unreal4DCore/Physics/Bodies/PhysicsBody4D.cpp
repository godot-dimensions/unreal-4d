#include "PhysicsBody4D.h"

#include "../Server/PhysicsServer4D.h"
#include "../KinematicCollision4D.h"
#include "Engine/World.h"
#include "PhysicsSettingsCore.h"

void UPhysicsBody4DComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return;
	}
	physicsServer4D->RegisterPhysicsBody(this);
}

void UPhysicsBody4DComponent::OnUnregister()
{
	Super::OnUnregister();

	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return;
	}
	physicsServer4D->UnregisterPhysicsBody(this);
}

FVector4 UPhysicsBody4DComponent::GetGravity() const
{
	// For now use the 3D gravity amount for 4D.
	const double globalGravityAmount = UPhysicsSettingsCore::Get()->DefaultGravityZ;
	const FVector4 gravity = FVector4(0.0, 0.0, globalGravityAmount, 0.0);
	// TODO: Add gravity from UArea4DComponents.
	return gravity;
}

FKinematicCollision4D UPhysicsBody4DComponent::MoveAndCollide(const FVector4& InMotion, const bool InTestOnly, const double InDeltaTime)
{
	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return FKinematicCollision4D();
	}
	return physicsServer4D->MoveAndCollide(this, InMotion, InTestOnly, InDeltaTime);
}
