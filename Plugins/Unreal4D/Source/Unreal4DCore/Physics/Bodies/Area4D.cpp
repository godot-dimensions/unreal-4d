#include "Area4D.h"

#include "../Server/PhysicsServer4D.h"
#include "Engine/World.h"

void UArea4DComponent::OnRegister()
{
	Super::OnRegister();
	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return;
	}
	physicsServer4D->RegisterArea(this);
}

void UArea4DComponent::OnUnregister()
{
	Super::OnUnregister();
	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return;
	}
	physicsServer4D->UnregisterArea(this);
}

void UArea4DComponent::MoveArea(const FVector4& InMotion)
{
	UWorld* world = GetWorld();
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world ? world->GetSubsystem<UPhysicsServer4DWorldSubsystem>() : nullptr;
	if (physicsServer4D == nullptr)
	{
		return;
	}
	physicsServer4D->MoveArea(this, InMotion);
}
