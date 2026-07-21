#include "Shape4DComponent.h"

#include "../../Bodies/CharacterBody4D.h"
#include "../../Bodies/CollisionObject4D.h"
#include "../../Bodies/DynamicBody4D.h"
#include "../../Bodies/StaticBody4D.h"
#include "../../Server/PhysicsServer4D.h"

void UShape4DComponent::OnRegister()
{
	Super::OnRegister();
	_ancestorCollisionObject = FindAncestorCollisionObject();
	if (_ancestorCollisionObject != nullptr)
	{
		_ancestorCollisionObject->RegisterCollisionShape(this);
		if (CollisionLayer < 0)
		{
			// Automatically infer the collision layer based on the body type.
			if (_ancestorCollisionObject->IsA<UStaticBody4DComponent>())
			{
				// This one is pretty straightforward.
				CollisionLayer = CollisionChannelToLayer(ECC_WorldStatic);
			}
			else if (_ancestorCollisionObject->IsA<UCharacterBody4DComponent>())
			{
				// Unreal uses "Pawn" to refer to both players and characters controlled by bots.
				CollisionLayer = CollisionChannelToLayer(ECC_Pawn);
			}
			else if (_ancestorCollisionObject->IsA<UDynamicBody4DComponent>())
			{
				// Unreal uses "ECC_PhysicsBody" for bodies simulated with rigid body dynamics, not to be confused with "ECC_WorldDynamic".
				CollisionLayer = CollisionChannelToLayer(ECC_PhysicsBody);
			}
			else
			{
				// Unreal uses the term "dynamic" to just mean "not static", regardless of simulation dynamics.
				CollisionLayer = CollisionChannelToLayer(ECC_WorldDynamic);
			}
		}
	}
}

void UShape4DComponent::OnUnregister()
{
	Super::OnUnregister();
	if (_ancestorCollisionObject != nullptr)
	{
		_ancestorCollisionObject->UnregisterCollisionShape(this);
		_ancestorCollisionObject = nullptr;
	}
}

void UShape4DComponent::OnAttachmentHierarchyChanged4D()
{
	Super::OnAttachmentHierarchyChanged4D();
	if (!IsRegistered())
	{
		return;
	}
	UCollisionObject4DComponent* newAncestorCollisionObject = FindAncestorCollisionObject();
	if (newAncestorCollisionObject == _ancestorCollisionObject)
	{
		return;
	}
	if (_ancestorCollisionObject != nullptr)
	{
		_ancestorCollisionObject->UnregisterCollisionShape(this);
	}
	_ancestorCollisionObject = newAncestorCollisionObject;
	if (_ancestorCollisionObject != nullptr)
	{
		_ancestorCollisionObject->RegisterCollisionShape(this);
	}
}

constexpr int32 UShape4DComponent::CollisionChannelToLayer(const ECollisionChannel InChannel)
{
	// Bit shift by InChannel bits.
	return 1 << (int32)InChannel;
}

ECollisionChannel UShape4DComponent::CollisionLayerToChannel(int32 InLayer)
{
	if (InLayer <= 0)
	{
		return ECC_WorldDynamic; // Fall back to WorldDynamic if the layer is invalid.
	}
	// Return the channel corresponding to the most significant bit set in InLayer.
    int32 index = 0;
    while (InLayer >>= 1)
	{
        index++;
    }
    return (ECollisionChannel)index;
}

UCollisionObject4DComponent* UShape4DComponent::FindAncestorCollisionObject() const
{
	USceneComponent* parent = GetAttachParent();
	while (parent != nullptr)
	{
		UScene4DComponent* parent4D = Cast<UScene4DComponent>(parent);
		if (unlikely(parent4D == nullptr))
		{
			break;
		}
		UCollisionObject4DComponent* co = Cast<UCollisionObject4DComponent>(parent4D);
		if (likely(co != nullptr))
		{
			return co;
		}
		parent = parent->GetAttachParent();
	}
	UWorld* world = GetWorld();
	ensure(world != nullptr);
	UPhysicsServer4DWorldSubsystem* physicsServer4D = world->GetSubsystem<UPhysicsServer4DWorldSubsystem>();
	ensure(physicsServer4D != nullptr);
	return physicsServer4D->GetOrCreateWorldStaticBody();
}

UCollisionObject4DComponent* UShape4DComponent::GetAncestorCollisionObject() const
{
	return _ancestorCollisionObject.Get();
}

FTransform4D UShape4DComponent::GetTransformToCollisionObject() const
{
	FTransform4D transformToColObj = GetLocalTransform4D();
	USceneComponent* parent = GetAttachParent();
	while (parent != nullptr && parent != _ancestorCollisionObject)
	{
		UScene4DComponent* parent4D = Cast<UScene4DComponent>(parent);
		if (unlikely(parent4D == nullptr))
		{
			break;
		}
		transformToColObj = parent4D->GetLocalTransform4D().Compose(transformToColObj);
		parent = parent->GetAttachParent();
	}
	return transformToColObj;
}

void UShape4DComponent::SetShapeData(UShape4DData* InNewShapeData4D)
{
	if (InNewShapeData4D == nullptr)
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UShape4DComponent::SetShapeData: InNewShapeData4D is null."));
		ensure(false);
		return;
	}
	if (InNewShapeData4D->GetOuter() == this)
	{
		Shape4D = InNewShapeData4D;
	}
	else
	{
		Shape4D = DuplicateObject<UShape4DData>(InNewShapeData4D, this);
	}
	UpdateComponentFromShape();
}

void UShape4DComponent::EnsureShapeDataIsOwned()
{
	if (IsValid(Shape4D) && Shape4D->GetOuter() != this)
	{
		Shape4D = DuplicateObject<UShape4DData>(Shape4D, this);
	}
}

bool UShape4DComponent::HasWorldPoint(const FVector4& InWorldPoint) const
{
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UShape4DComponent::HasWorldPoint: Shape4D is null or invalid."));
		ensure(false);
		return false;
	}
	return Shape4D->HasLocalPoint(GetGlobalTransform4D().InverseTransformPosition(InWorldPoint));
}

FRect4D UShape4DComponent::GetRectBoundsLocal4D(const FTransform4D& InToTarget) const
{
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UShape4DComponent::GetRectBoundsLocal4D: Shape4D is null or invalid."));
		ensure(false);
		return FRect4D();
	}
	return Shape4D->GetRectBoundsLocal4D(InToTarget);
}

FRaycastResult4D UShape4DComponent::RaycastIntersectsLocal4D(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const double InMaxDistance, const bool bInInsideIsZero)
{
	if (Shape4D == nullptr || !IsValid(Shape4D) || !Shape4D->IsValidLowLevel())
	{
		UE_LOG(LogUnreal4DCore, Error, TEXT("UShape4DComponent::RaycastIntersectsLocal4D: Shape4D is null or invalid."));
		ensure(false);
		return FRaycastResult4D();
	}
	FRaycastResult4D result = Shape4D->RaycastIntersectsLocal4D(InLocalRayOrigin, InLocalRayDirection, InMaxDistance, bInInsideIsZero);
	if (result.bHit)
	{
		result.ShapeComponent = this;
	}
	return result;
}

void UShape4DComponent::PostInitProperties()
{
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
		ensure(Shape4D != nullptr && IsValid(Shape4D)); // Derived classes MUST initialize a shape in their construction logic.
	}
	Super::PostInitProperties();
}

void UShape4DComponent::PostLoad()
{
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
		ensure(Shape4D != nullptr && IsValid(Shape4D)); // Derived classes MUST initialize a shape in their construction logic.
	}
	Super::PostLoad();
}
