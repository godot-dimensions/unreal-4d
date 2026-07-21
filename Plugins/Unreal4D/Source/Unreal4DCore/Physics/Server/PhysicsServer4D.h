#pragma once

#include "PhysicsEngine4D.h"
#include "Subsystems/WorldSubsystem.h"

#include "PhysicsServer4D.generated.h"

constexpr double PHYSICS_4D_SAFE_MARGIN = 0.000000000001;

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

	// These are scoped to the things that exist in the UWorld.
	TArray<UArea4DComponent*> _areaComponents;
	TArray<UPhysicsBody4DComponent*> _physicsBodyComponents;

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

	TArray<UArea4DComponent*> GetAreaComponents() const;
	TArray<UPhysicsBody4DComponent*> GetPhysicsBodyComponents() const;

	UCollisionObject4DComponent* GetOrCreateWorldStaticBody();
};
