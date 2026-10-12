#pragma once

#include "PhysicsEngine4D.h"
#include "../CollisionManifold4D.h"
#include "Subsystems/WorldSubsystem.h"

#include "PhysicsServer4D.generated.h"

constexpr double PHYSICS_4D_SAFE_MARGIN = 0.000000000001;

class UPrimitive4DComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrimitive4DRegistrationDelegate, UPrimitive4DComponent*, InPrimitive);

UENUM()
enum class EPhysicsTickPolicy4D : uint8
{
	// Do not tick physics automatically. Physics operations will only occur externally,
	// such as through `MoveAndCollide()` calls, or through manual calls to `PhysicsTick()`.
	Disabled,
	// Tick physics at a fixed time step, which is independent of the render frame rate.
	// This results in more deterministic physics behavior, and is better for multiplayer or competitive games.
	FixedTimeStep,
	// Tick physics once per world tick (per render frame, if rendering frames), which may be variable depending on the render frame rate.
	// This results in smoother-looking physics behavior, and may be preferred for single-player or casual games.
	PerWorldTick,
};

UCLASS()
class UNREAL4DCORE_API UPhysicsServer4DWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UPhysicsEngine4D>> _physicsEngines;

	UPROPERTY(Transient)
	TObjectPtr<UPhysicsEngine4D> _currentPhysicsEngine = nullptr;
	FName _currentPhysicsEngineName = NAME_None;

	// These 3 transient arrays are scoped to the things that exist in the UWorld.
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UArea4DComponent>> _areaComponents;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPhysicsBody4DComponent>> _physicsBodyComponents;

	// Primitives are all non-body things that can participate in physics,
	// including meshes (UMesh4DComponent) and shapes (UShape4DComponent).
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UPrimitive4DComponent>> _primitiveColliders;

	UPhysicsEngine4D* GetPhysicsEngine(const FName& InName) const;

	double _fixedTimeStepAccumulator = 0.0;

	UPROPERTY(Transient)
	TObjectPtr<UCollisionObject4DComponent> _worldStaticBodyForBodylessShapes = nullptr;

public:
	// UWorldSubsystem setup and initialization hooks.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type InWorldType) const override;

	// UTickableWorldSubsystem ticking hooks.
	virtual void Tick(float InDeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Server")
	EPhysicsTickPolicy4D TickPolicy = EPhysicsTickPolicy4D::FixedTimeStep;

	// These two properties are only used when the tick policy is EPhysicsTickPolicy4D::FixedTimeStep.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Server", meta = (EditCondition = "TickPolicy == EPhysicsTickPolicy4D::FixedTimeStep"))
	double FixedTimeStep = 1.0 / 60.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Server", meta = (EditCondition = "TickPolicy == EPhysicsTickPolicy4D::FixedTimeStep"))
	int32 MaxPhysicsTicksPerWorldTick = 8;

	// This can be called manually to perform custom ticking, but usually should not be called externally.
	void PhysicsTick(const double InDeltaTime);

	FKinematicCollision4D MoveAndCollide(UPhysicsBody4DComponent* InBodyNode, FVector4 InMotion, bool InTestOnly = false, double InDeltaTime = -1.0);
	void MoveArea(UArea4DComponent* InAreaComponent, FVector4 InMotion);

	void RegisterPrimitive(UPrimitive4DComponent* InPrimitiveCollider);
	void UnregisterPrimitive(UPrimitive4DComponent* InPrimitiveCollider);

	// RegisterPrimitive/UnregisterPrimitive notify synchronously after registry membership changes.
	// Component OnRegister/OnUnregister may still be running; defer component creation or destruction.
	UPROPERTY(Transient, BlueprintAssignable, Category = "Unreal 4D|Physics|Server")
	FPrimitive4DRegistrationDelegate OnPrimitiveRegistered;

	UPROPERTY(Transient, BlueprintAssignable, Category = "Unreal 4D|Physics|Server")
	FPrimitive4DRegistrationDelegate OnPrimitiveUnregistered;

	// Appends hits from registered Primitive4D colliders. InAny stops after the first hit.
	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Physics|Server")
	bool QueryPointCollision(const FVector4& InPoint, TArray<FCollisionManifold4D>& OutCollisions, const bool InAny = false) const;

	void RegisterArea(UArea4DComponent* InAreaComponent);
	void UnregisterArea(UArea4DComponent* InAreaComponent);

	void RegisterPhysicsBody(UPhysicsBody4DComponent* InPhysicsBodyComponent);
	void UnregisterPhysicsBody(UPhysicsBody4DComponent* InPhysicsBodyComponent);

	void RegisterPhysicsEngine(const FName& InName, UPhysicsEngine4D* InEngine);
	void UnregisterPhysicsEngine(const FName& InName);
	void SetPhysicsEngine(const FName& InName);
	TArray<FName> GetPhysicsEngineNames() const;

	FName GetCurrentPhysicsEngineName() const;
	void SetCurrentPhysicsEngineName(const FName& InName);

	// Return snapshots so registration changes during callbacks do not invalidate iteration.
	// Resolve each weak reference before use; components may have been destroyed since the snapshot.
	TArray<TWeakObjectPtr<UPrimitive4DComponent>> GetRegisteredColliders() const;
	TArray<TWeakObjectPtr<UArea4DComponent>> GetAreaComponents() const;
	TArray<TWeakObjectPtr<UPhysicsBody4DComponent>> GetPhysicsBodyComponents() const;

	UCollisionObject4DComponent* GetOrCreateWorldStaticBody();
};
