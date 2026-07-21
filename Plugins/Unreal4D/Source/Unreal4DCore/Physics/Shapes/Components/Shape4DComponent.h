#pragma once

#include "../../../Scene/Components/Primitive4DComponent.h"
#include "../Data/Shape4DData.h"

#include "Shape4DComponent.generated.h"

class UPhysicalMaterial;
class UCollisionObject4DComponent;

// UShape4DComponent is a base component class that can hold a shape derived from UShape4DData.
// UShape4DComponent may not be used directly, only derived classes may be used, such as
// UBoxShape4DComponent, USphereShape4DComponent, UCapsuleShape4DComponent, and more.
UCLASS(Abstract, Category = "Unreal 4D|Physics|Shape")
class UNREAL4DCORE_API UShape4DComponent : public UPrimitive4DComponent
{
	GENERATED_BODY()

	UCollisionObject4DComponent* FindAncestorCollisionObject() const;
	TWeakObjectPtr<UCollisionObject4DComponent> _ancestorCollisionObject;

protected:
	UPROPERTY(Instanced)
	TObjectPtr<UShape4DData> Shape4D;
	void EnsureShapeDataIsOwned();

	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void OnAttachmentHierarchyChanged4D() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	TObjectPtr<UPhysicalMaterial> PhysicalMaterial4D = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	int32 CollisionLayer = -1; // Bitmask for the collision layer this shape is on (populated from Unreal Collision Channels).

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unreal 4D|Physics|Shape")
	int32 CollisionMask = MAX_int32; // Bitmask for the collision layers this shape collides with.

	static constexpr int32 CollisionChannelToLayer(const ECollisionChannel InChannel);
	static ECollisionChannel CollisionLayerToChannel(int32 InLayer);
	UCollisionObject4DComponent* GetAncestorCollisionObject() const;
	FTransform4D GetTransformToCollisionObject() const;

	virtual UShape4DData* GetShapeData() const override { return Shape4D; }
	void SetShapeData(UShape4DData* InNewShapeData4D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, Category = "Unreal 4D|Physics|Shape")
	virtual bool HasWorldPoint(const FVector4& InWorldPoint) const;

	virtual FRect4D GetRectBoundsLocal4D(const FTransform4D& InToTarget) const override;
	virtual FRaycastResult4D RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance = 1e300, const bool bInInsideIsZero = false) override;

	virtual void UpdateComponentFromShape() PURE_VIRTUAL(UShape4DComponent::UpdateComponentFromShape, );
	virtual void UpdateShapeFromComponent() PURE_VIRTUAL(UShape4DComponent::UpdateShapeFromComponent, );
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
};
