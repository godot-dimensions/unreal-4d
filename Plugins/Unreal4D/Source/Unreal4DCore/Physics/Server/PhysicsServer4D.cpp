#include "PhysicsServer4D.h"

#include "../Bodies/StaticBody4D.h"
#include "AxisAlignedBoxPhysicsEngine4D.h"
#include "../../Scene/Components/Primitive4DComponent.h"

void UPhysicsServer4DWorldSubsystem::Initialize(FSubsystemCollectionBase& InCollection)
{
	Super::Initialize(InCollection);
	// Register physics engines.
	UAxisAlignedBoxPhysicsEngine4D* aabbPhysicsEngine = NewObject<UAxisAlignedBoxPhysicsEngine4D>(
		this, TEXT("Unreal4DAxisAlignedBoxPhysicsEngine4D"));
	RegisterPhysicsEngine(TEXT("AxisAlignedBox"), aabbPhysicsEngine);
	const UWorld* world = GetWorld();
	if (ensure(world != nullptr))
	{
		// Disable ticking by default for editor worlds. Ticking only
		// happens in game worlds and Play-In-Editor (PIE) worlds.
		if (!world->IsGameWorld())
		{
			TickPolicy = EPhysicsTickPolicy4D::Disabled;
		}
	}
}

void UPhysicsServer4DWorldSubsystem::Deinitialize()
{
	SetCurrentPhysicsEngineName(NAME_None);
	_physicsEngines.Empty();
	_areaComponents.Empty();
	_physicsBodyComponents.Empty();
	_primitiveColliders.Empty();
	if (_worldStaticBodyForBodylessShapes != nullptr)
	{
		_worldStaticBodyForBodylessShapes->DestroyComponent();
		_worldStaticBodyForBodylessShapes = nullptr;
	}
	Super::Deinitialize();
}

bool UPhysicsServer4DWorldSubsystem::DoesSupportWorldType(const EWorldType::Type InWorldType) const
{
	switch (InWorldType)
	{
		case EWorldType::Game:
		case EWorldType::Editor:
		case EWorldType::PIE:
		case EWorldType::EditorPreview:
		{
			return true;
		}
		default:
		{
			return false;
		}
	}
}

void UPhysicsServer4DWorldSubsystem::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
	switch (TickPolicy)
	{
		case EPhysicsTickPolicy4D::Disabled:
		{
			// Do nothing, it's disabled. However, this shouldn't be reached because `IsTickable()` returns false.
			ensure(false);
		} break;
		case EPhysicsTickPolicy4D::FixedTimeStep:
		{
			_fixedTimeStepAccumulator += InDeltaTime;
			int32 tickIndex = 0;
			if (!ensure(FixedTimeStep > 0.0))
			{
				UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Invalid FixedTimeStep, must be greater than 0.0. Skipping physics tick."));
				return;
			}
			ensure(MaxPhysicsTicksPerWorldTick > 0); // No need for an if on this one, the fallback is just that it won't run.
			while (_fixedTimeStepAccumulator >= FixedTimeStep && tickIndex < MaxPhysicsTicksPerWorldTick)
			{
				_fixedTimeStepAccumulator -= FixedTimeStep;
				PhysicsTick(FixedTimeStep);
				tickIndex++;
			}
			if (tickIndex >= MaxPhysicsTicksPerWorldTick)
			{
				// If the game has a crazy lag spike and freezes for, say, a minute,
				// we don't want to run the game at a hyper-fast speed for several seconds.
				// Instead, just abandon any accumulated time and reset the accumulator to 0.
				// This means that a frozen game, when unfrozen, will lose time rather than catching up.
				_fixedTimeStepAccumulator = 0.0;
			}
		} break;
		case EPhysicsTickPolicy4D::PerWorldTick:
		{
			_fixedTimeStepAccumulator = 0.0;
			PhysicsTick(InDeltaTime);
		} break;
	}
}

TStatId UPhysicsServer4DWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPhysicsServer4DWorldSubsystem, STATGROUP_Tickables);
}

bool UPhysicsServer4DWorldSubsystem::IsTickable() const
{
	return TickPolicy != EPhysicsTickPolicy4D::Disabled;
}

bool UPhysicsServer4DWorldSubsystem::IsTickableInEditor() const
{
	if (!IsTickable())
	{
		return false;
	}
	// The `Initialize()` function disables ticking for editor worlds on setup, which is
	// ALMOST sufficient, but it does not account for pausing Play-In-Editor (PIE) worlds.
	const UWorld* world = GetWorld();
	return world != nullptr && !world->IsGameWorld();
}

void UPhysicsServer4DWorldSubsystem::PhysicsTick(const double InDeltaTime)
{
	//UE_LOG(LogUnreal4DCore, Display, TEXT("UPhysicsServer4DWorldSubsystem: PhysicsTick called with InDeltaTime = %f"), InDeltaTime);
	if (_currentPhysicsEngine == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UPhysicsServer4DWorldSubsystem: No physics engine is set. Skipping physics tick."));
		ensure(false);
		return;
	}
	// Set LastPhysicsDeltaTime in all physics bodies.
	for (const TWeakObjectPtr<UPhysicsBody4DComponent>& bodyObjPtr : _physicsBodyComponents)
	{
		if (bodyObjPtr.IsValid())
		{
			bodyObjPtr->LastPhysicsDeltaTime = InDeltaTime;
		}
	}
	// Hand off control to the current physics engine to process the physics tick.
	_currentPhysicsEngine->PhysicsProcess(InDeltaTime);
}

FKinematicCollision4D UPhysicsServer4DWorldSubsystem::MoveAndCollide(UPhysicsBody4DComponent* InBodyNode, FVector4 InMotion, bool InTestOnly, double InDeltaTime)
{
	if (InBodyNode == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot move a null UPhysicsBody4DComponent."));
		ensure(false);
		return FKinematicCollision4D();
	}
	if (_currentPhysicsEngine == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: No physics engine is set. Cannot move physics body."));
		ensure(false);
		return FKinematicCollision4D();
	}
	return _currentPhysicsEngine->MoveAndCollide(InBodyNode, InMotion, InTestOnly, InDeltaTime);
}

void UPhysicsServer4DWorldSubsystem::MoveArea(UArea4DComponent* InAreaComponent, FVector4 InMotion)
{
	if (InAreaComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot move a null UArea4DComponent."));
		ensure(false);
		return;
	}
	if (_currentPhysicsEngine == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: No physics engine is set. Cannot move area."));
		ensure(false);
		return;
	}
	_currentPhysicsEngine->MoveArea(InAreaComponent, InMotion);
}

void UPhysicsServer4DWorldSubsystem::RegisterPrimitive(UPrimitive4DComponent* InPrimitiveCollider)
{
	if (IsValid(InPrimitiveCollider) && !_primitiveColliders.Contains(InPrimitiveCollider))
	{
		_primitiveColliders.Add(InPrimitiveCollider);
		OnPrimitiveRegistered.Broadcast(InPrimitiveCollider);
	}
}

void UPhysicsServer4DWorldSubsystem::UnregisterPrimitive(UPrimitive4DComponent* InPrimitiveCollider)
{
	if (_primitiveColliders.RemoveSingleSwap(InPrimitiveCollider) > 0)
	{
		OnPrimitiveUnregistered.Broadcast(InPrimitiveCollider);
	}
}

bool UPhysicsServer4DWorldSubsystem::QueryPointCollision(const FVector4& InPoint, TArray<FCollisionManifold4D>& OutCollisions, const bool InAny) const
{
	// TODO: Use spatial partitioning to avoid querying every collider.
	bool result = false;
	for (const TWeakObjectPtr<UPrimitive4DComponent>& colliderRef : _primitiveColliders)
	{
		if (const UPrimitive4DComponent* collider = colliderRef.Get())
		{
			FCollisionManifold4D collision;
			if (collider->QueryPointCollision(InPoint, collision))
			{
				ensureAlways(collision.IsValid());
				OutCollisions.Add(collision);
				result = true;
				if (InAny)
				{
					break;
				}
			}
		}
	}
	return result;
}

void UPhysicsServer4DWorldSubsystem::RegisterArea(UArea4DComponent* InAreaComponent)
{
	if (InAreaComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot register a null UArea4DComponent."));
		ensure(false);
		return;
	}
	_areaComponents.Add(InAreaComponent);
}

void UPhysicsServer4DWorldSubsystem::UnregisterArea(UArea4DComponent* InAreaComponent)
{
	if (InAreaComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot unregister a null UArea4DComponent."));
		ensure(false);
		return;
	}
	_areaComponents.Remove(InAreaComponent);
}

void UPhysicsServer4DWorldSubsystem::RegisterPhysicsBody(UPhysicsBody4DComponent* InPhysicsBodyComponent)
{
	if (InPhysicsBodyComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot register a null UPhysicsBody4DComponent."));
		ensure(false);
		return;
	}
	_physicsBodyComponents.Add(InPhysicsBodyComponent);
}

void UPhysicsServer4DWorldSubsystem::UnregisterPhysicsBody(UPhysicsBody4DComponent* InPhysicsBodyComponent)
{
	if (InPhysicsBodyComponent == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem: Cannot unregister a null UPhysicsBody4DComponent."));
		ensure(false);
		return;
	}
	_physicsBodyComponents.Remove(InPhysicsBodyComponent);
}

UPhysicsEngine4D* UPhysicsServer4DWorldSubsystem::GetPhysicsEngine(const FName& InName) const
{
	if (_physicsEngines.Num() == 0)
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UPhysicsServer4DWorldSubsystem::GetPhysicsEngine: There are no registered physics engines. Returning nullptr."));
		return nullptr;
	}
	if (_physicsEngines.Contains(InName))
	{
		return _physicsEngines[InName];
	}
	// Fallback to the first registered physics engine. If the name is empty,
	// treat it as "auto" and do not print a warning. Else, print a warning.
	if (!InName.IsNone())
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UPhysicsServer4DWorldSubsystem::GetPhysicsEngine: Physics engine '%s' not registered. Using the first registered engine."), *InName.ToString());
	}
	return _physicsEngines.begin()->Value;
}

void UPhysicsServer4DWorldSubsystem::RegisterPhysicsEngine(const FName& InName, UPhysicsEngine4D* InEngine)
{
	if (_physicsEngines.Contains(InName))
	{
		UE_LOG(LogUnreal4DCore, Warning, TEXT("UPhysicsServer4DWorldSubsystem::RegisterPhysicsEngine: Physics engine '%s' already registered. The existing engine will be replaced."), *InName.ToString());
	}
	_physicsEngines.FindOrAdd(InName) = InEngine;
	// If no current physics engine is set, set this one as the current engine.
	if (_currentPhysicsEngine == nullptr)
	{
		SetCurrentPhysicsEngineName(InName);
	}
}

void UPhysicsServer4DWorldSubsystem::UnregisterPhysicsEngine(const FName& InName)
{
	_physicsEngines.Remove(InName);
	if (_currentPhysicsEngineName == InName)
	{
		if (_physicsEngines.Num() == 0)
		{
			_currentPhysicsEngine = nullptr;
			_currentPhysicsEngineName = NAME_None;
		}
		else
		{
			_currentPhysicsEngineName = _physicsEngines.begin()->Key;
			_currentPhysicsEngine = _physicsEngines.begin()->Value;
		}
	}
}

TArray<FName> UPhysicsServer4DWorldSubsystem::GetPhysicsEngineNames() const
{
	// TMap doesn't have a keys() method, so we have to do this manually.
	TArray<FName> engineNames;
	if (_physicsEngines.Num() == 0)
	{
		return engineNames;
	}
	engineNames.SetNum(_physicsEngines.Num());
	int i = 0;
	for (const TPair<FName, TObjectPtr<UPhysicsEngine4D>>& nameEnginePair : _physicsEngines)
	{
		engineNames[i] = FName(nameEnginePair.Key);
		i++;
	}
	return engineNames;
}

FName UPhysicsServer4DWorldSubsystem::GetCurrentPhysicsEngineName() const
{
	return _currentPhysicsEngineName;
}

void UPhysicsServer4DWorldSubsystem::SetCurrentPhysicsEngineName(const FName& InName)
{
	if (InName == _currentPhysicsEngineName)
	{
		return;
	}
	if (InName.IsNone())
	{
		_currentPhysicsEngine = nullptr;
		_currentPhysicsEngineName = NAME_None;
		return;
	}
	if (_physicsEngines.Num() == 0)
	{
		_currentPhysicsEngine = nullptr;
		_currentPhysicsEngineName = NAME_None;
		UE_LOG(LogUnreal4DCore, Error, TEXT("UPhysicsServer4DWorldSubsystem::SetCurrentPhysicsEngineName: There are no registered physics engines. Cannot set current physics engine."));
		return;
	}
	if (_physicsEngines.Contains(InName))
	{
		_currentPhysicsEngineName = InName;
		_currentPhysicsEngine = _physicsEngines[InName];
		return;
	}
	_currentPhysicsEngineName = _physicsEngines.begin()->Key;
	_currentPhysicsEngine = _physicsEngines.begin()->Value;
	UE_LOG(LogUnreal4DCore, Warning, TEXT("UPhysicsServer4DWorldSubsystem::SetCurrentPhysicsEngineName: Physics engine '%s' not registered. The first registered engine '%s' will be used as a fallback."), *InName.ToString(), *_currentPhysicsEngineName.ToString());
}

TArray<TWeakObjectPtr<UPrimitive4DComponent>> UPhysicsServer4DWorldSubsystem::GetRegisteredColliders() const
{
	return _primitiveColliders;
}

TArray<TWeakObjectPtr<UArea4DComponent>> UPhysicsServer4DWorldSubsystem::GetAreaComponents() const
{
	return _areaComponents;
}

TArray<TWeakObjectPtr<UPhysicsBody4DComponent>> UPhysicsServer4DWorldSubsystem::GetPhysicsBodyComponents() const
{
	return _physicsBodyComponents;
}

UCollisionObject4DComponent* UPhysicsServer4DWorldSubsystem::GetOrCreateWorldStaticBody()
{
	if (_worldStaticBodyForBodylessShapes != nullptr)
	{
		return _worldStaticBodyForBodylessShapes;
	}
	UWorld* world = GetWorld();
	ensure(world != nullptr);
	UStaticBody4DComponent* worldStaticBody = NewObject<UStaticBody4DComponent>(this, TEXT("Unreal4DWorldStaticBody4D"));
	ensure(worldStaticBody != nullptr);
	worldStaticBody->RegisterComponentWithWorld(world);
	_worldStaticBodyForBodylessShapes = worldStaticBody;
	return _worldStaticBodyForBodylessShapes;
}
