#include "Scene4DComponent.h"

#include "../../Render/RenderingServer4D.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/World.h"

bool UScene4DComponent::bReportNon4DParentErrors = true;

UScene4DComponent::UScene4DComponent(const FObjectInitializer& Init)
	: Super(Init)
{
	SetGenerateOverlapEvents(false);
}

UScene4DComponent* UScene4DComponent::GetParent4DComponent() const
{
	USceneComponent* parent = GetAttachParent();
	if (parent == nullptr)
	{
		// Not attached to anything, so the local transform is the global transform.
		return nullptr;
	}
	UScene4DComponent* parent4D = Cast<UScene4DComponent>(parent);
	if (parent4D == nullptr)
	{
		if (bReportNon4DParentErrors)
		{
			UE_LOG(LogUnreal4DCore, Error, TEXT("UScene4DComponent::GetParent4DComponent: Component '%s' has attachment parent '%s', which is not a UScene4DComponent."), *GetPathName(), *parent->GetPathName());
		}
		// A non-4D parent is treated the same as no parent: this is the root of the 4D hierarchy.
		return nullptr;
	}
	return parent4D;
}

UScene4DComponent* UScene4DComponent::FindParent4DComponent(USceneComponent* InComponent)
{
	UScene4DComponent* result = nullptr;
	if (InComponent != nullptr)
	{
		USceneComponent* parent = InComponent->GetAttachParent();
		UScene4DComponent* parent4D = Cast<UScene4DComponent>(parent);
		result = parent4D != nullptr ? parent4D : UScene4DComponent::FindParent4DComponent(parent);
	}
	return result;
}

// Transform altering methods.

// Syncs this component's 4D transform from its Blueprint archetype (template).
// Only use this for non-root components in Blueprint instances.
void UScene4DComponent::SyncTransform4DFromArchetype()
{
	// Get the archetype (Blueprint template) version of this component, if this is part of a Blueprint.
	UScene4DComponent* archetypeComponent = Cast<UScene4DComponent>(GetArchetype());
	if (archetypeComponent == nullptr)
	{
		return;
	}
	// Every UObject has an archetype, including level-created components. Only Blueprint
	// component templates define transforms that should be inherited by placed instances.
	if (archetypeComponent->HasAnyFlags(RF_ClassDefaultObject) || archetypeComponent->GetTypedOuter<UBlueprintGeneratedClass>() == nullptr)
	{
		return;
	}
	SetLocalTransform4D(archetypeComponent->GetLocalTransform4D());
}

void UScene4DComponent::UpdatedTransform4D()
{
	// Update 3D transform so that the 3D editor shows the correct position.
	// But use the orthonormalized version to avoid skewing in 3D space.
	// The resulting 3D transform will have a uniform scale of 1.0 in all axes.
	UWorld* world = GetWorld();
	if (world == nullptr)
	{
		return;
	}
	// Get the camera transforms from the rendering server, if it exists, because the
	// 3D transform depends on the slice of 4D space that the camera is currently in.
	FTransform4D cameraTransform4D = FTransform4D::Identity();
	FTransform cameraTransform3D = FTransform::Identity;
	URenderingServer4D* renderingServer4D = world->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		cameraTransform4D = renderingServer4D->GetCurrentCameraTransform4D();
		cameraTransform3D = renderingServer4D->GetCurrentCameraTransform3D();
		// Quick fix (added by Timofei):
		// Handles the case where this 4D component is the root of the current camera.
		//
		// Without this fix, the 3D transform of the current camera was always zero
		// (both position and rotation), since it was being computed relative to itself.
		//
		// This currently also serves as a workaround to generate a 3D transform from
		// the 4D transform of the camera.
		//
		// Note: The overall approach of converting a 4D transform (especially rotation)
		// into a 3D transform is not well-defined in all cases and likely needs a more
		// robust design.
		if (UCamera4DComponent* currentCamera = renderingServer4D->GetCurrentCamera())
		{
			AActor* cameraOwner = currentCamera->GetOwner();
			if (cameraOwner && cameraOwner->GetRootComponent() == this)
			{
				FTransform transform3D = GetGlobalTransform4D().To3D(true);
				transform3D.SetScale3D(FVector(1, 1, 1));
				SetWorldTransform(transform3D);

				NotifyTransform4DChanged();
				return;
			}
		}
	}
	// Transform the object's 4D world space transform to camera-relative 4D space,
	// then convert the 4D transform to 3D, then apply the camera's 3D transform to
	// get world space. This is required to handle camera rotations into the W axis.
	const FTransform4D relToCamera4D = cameraTransform4D.Inverse().Compose(GetGlobalTransform4D());
	// IMPORTANT: This is Unreal's 3D transform, this uses `child * parent` order!
	FTransform transform3D = relToCamera4D.To3D(true) * cameraTransform3D;
	transform3D.SetScale3D(FVector(1.0f, 1.0f, 1.0f));
	SetWorldTransform(transform3D);
	// Notify listeners that the 4D transform has changed.
	NotifyTransform4DChanged();
	// Propagate transform update to children.
	const TArray<TObjectPtr<USceneComponent>>& children = GetAttachChildren();
	for (USceneComponent* child : children)
	{
		if (UScene4DComponent* child4D = Cast<UScene4DComponent>(child))
		{
			child4D->UpdatedTransform4D();
		}
	}
}

void UScene4DComponent::NotifyTransform4DChanged()
{
	OnTransform4DChanged();

	OnTransform4DChangedDelegate.Broadcast(this);
}

void UScene4DComponent::ApplyScale(const FVector4& InScale)
{
	_localPosition4D *= InScale;
	SetLocalBasis4D(UBasis4D::Basis4D_ScaledGlobal(_localBasis4D, InScale));
	UpdatedTransform4D();
}

void UScene4DComponent::TranslateLocal(const FVector4& InTranslation)
{
	_localPosition4D += _localBasis4D.TransformFVector4(InTranslation);
	UpdatedTransform4D();
}

void UScene4DComponent::LookAt(const FVector4& InGlobalTarget, const FVector4& InGlobalUp)
{
	FTransform4D globalTransform = GetGlobalTransform4D();
	const FVector4 relativeTarget = InGlobalTarget - globalTransform.Origin;
	const FVector4 absScale = UBasis4D::Basis4D_GetScaleAbs(globalTransform.Basis);
	const FVector4 globalBasisW = FVector4(globalTransform.Basis.M[3][0], globalTransform.Basis.M[3][1], globalTransform.Basis.M[3][2], globalTransform.Basis.M[3][3]);
	globalTransform.Basis = UBasis4D::Basis4D_LookingAt(relativeTarget, globalBasisW, InGlobalUp);
	globalTransform.Basis = UBasis4D::Basis4D_ScaledLocal(globalTransform.Basis, absScale);
	SetGlobalTransform4D(globalTransform);
}

// Rotation altering methods.

void UScene4DComponent::RotateRadians(const FEuler4D& InEulerRadians)
{
	FMatrix rot = InEulerRadians.RadiansToBasis();
	SetLocalBasis4D(UBasis4D::Basis4D_Compose(rot, GetLocalBasis4D()));
}

void UScene4DComponent::RotateLocalRadians(const FEuler4D& InLocalEulerRadians)
{
	FMatrix rot = InLocalEulerRadians.RadiansToBasis();
	SetLocalBasis4D(UBasis4D::Basis4D_Compose(GetLocalBasis4D(), rot));
}

void UScene4DComponent::RotateDegrees(const FEuler4D& InEulerDegrees)
{
	RotateRadians(InEulerDegrees.DegToRad());
}

void UScene4DComponent::RotateLocalDegrees(const FEuler4D& InLocalEulerDegrees)
{
	RotateLocalRadians(InLocalEulerDegrees.DegToRad());
}

// Local transform and basis.

void UScene4DComponent::SetLocalTransform4D(const FTransform4D& InTransform)
{
	_localPosition4D = InTransform.Origin;
	SetLocalBasis4D(InTransform.Basis);
}

void UScene4DComponent::SetLocalBasis4D(const FMatrix& InBasis)
{
	_localBasis4D = InBasis;
	_localRotationDegrees4D = FEuler4D::RadiansFromBasis(InBasis).RadToDeg();
	_localScale4D = UBasis4D::Basis4D_GetScale(InBasis);
	_localUniformScale4D = UBasis4D::Basis4D_GetUniformScale(InBasis);
	UpdatedTransform4D();
}

void UScene4DComponent::SetLocalTransform4Dfrom3D(const FTransform& InTransform3D)
{
	UWorld* world = GetWorld();
	if (world == nullptr)
	{
		return;
	}
	// Get the camera transforms from the rendering server, if it exists, because the
	// 3D transform depends on the slice of 4D space that the camera is currently in.
	FTransform4D cameraTransform4D = FTransform4D::Identity();
	FTransform cameraTransform3D = FTransform::Identity;
	URenderingServer4D* renderingServer4D = world->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D != nullptr)
	{
		cameraTransform4D = renderingServer4D->GetCurrentCameraTransform4D();
		cameraTransform3D = renderingServer4D->GetCurrentCameraTransform3D();
	}
	// Transform the object's 3D world space transform to camera-relative 3D space,
	// then convert the 3D transform to 4D, then apply the camera's 4D transform to
	// get world space. This is required to handle camera rotations into the W axis.
	// IMPORTANT: This is Unreal's 3D transform, this uses `child * parent` order!
	const FTransform relToCamera3D = InTransform3D * cameraTransform3D.Inverse();
	FTransform4D transform4D = cameraTransform4D.Compose(FTransform4D::From3D(relToCamera3D));
	// A 3D transform is an unreliable and incomplete source of truth for scale,
	// so let's only grab a uniform scale from the 3D transform.
	const double uniformScale3D = relToCamera3D.GetScale3D().GetMax();
	transform4D.Basis = UBasis4D::Basis4D_WithUniformScale(transform4D.Basis, uniformScale3D);
	SetLocalTransform4D(transform4D);
}

// Local transform components.

void UScene4DComponent::SetLocalPosition4D(const FVector4& InPosition)
{
	_localPosition4D = InPosition;
	UpdatedTransform4D();
}

void UScene4DComponent::SetLocalRotationRadians4D(const FEuler4D& InRotationEulerRadians)
{
	_localRotationDegrees4D = InRotationEulerRadians.RadToDeg();
	_localBasis4D = UBasis4D::Basis4D_ScaledLocal(InRotationEulerRadians.RadiansToBasis(), _localScale4D);
	UpdatedTransform4D();
}

void UScene4DComponent::SetLocalRotationDegrees4D(const FEuler4D& InRotationEulerDegrees)
{
	_localRotationDegrees4D = InRotationEulerDegrees;
	_localBasis4D = UBasis4D::Basis4D_ScaledLocal(InRotationEulerDegrees.DegToRad().RadiansToBasis(), _localScale4D);
	UpdatedTransform4D();
}

void UScene4DComponent::SetLocalScale4D(const FVector4& InScale)
{
	_localScale4D = InScale;
	_localBasis4D = UBasis4D::Basis4D_WithScale(_localBasis4D, InScale);
	_localUniformScale4D = UBasis4D::Basis4D_GetUniformScale(_localBasis4D);
	UpdatedTransform4D();
}

void UScene4DComponent::SetLocalUniformScale4D(const double InUniformScale)
{
	_localUniformScale4D = InUniformScale;
	const double absScale = FMath::Abs(InUniformScale);
	// The canonical negative scale is only on the W axis.
	_localScale4D = FVector4(absScale, absScale, absScale, InUniformScale);
	_localBasis4D = UBasis4D::Basis4D_WithUniformScale(_localBasis4D, InUniformScale);
	UpdatedTransform4D();
}

// Global transform and basis.

FTransform4D UScene4DComponent::GetGlobalTransform4D() const
{
	const UScene4DComponent* parent4D = GetParent4DComponent();
	if (parent4D == nullptr)
	{
		// Not attached to anything, so the local transform is the global transform.
		return FTransform4D(_localBasis4D, _localPosition4D);
	}
	return parent4D->GetGlobalTransform4D().Compose(FTransform4D(_localBasis4D, _localPosition4D));
}

FMatrix UScene4DComponent::GetGlobalBasis4D() const
{
	const UScene4DComponent* parent4D = GetParent4DComponent();
	if (parent4D == nullptr)
	{
		// Not attached to anything, so the local transform is the global transform.
		return _localBasis4D;
	}
	return UBasis4D::Basis4D_Compose(parent4D->GetGlobalBasis4D(), _localBasis4D);
}

void UScene4DComponent::SetGlobalTransform4D(const FTransform4D& InTransform)
{
	const UScene4DComponent* parent4D = GetParent4DComponent();
	if (parent4D == nullptr)
	{
		// Not attached to anything, so the local transform is the global transform.
		_localPosition4D = InTransform.Origin;
		SetLocalBasis4D(InTransform.Basis); // Will call `UpdatedTransform4D()`.
		return;
	}
	const FTransform4D localTransform4D = parent4D->GetGlobalTransform4D().Inverse() * InTransform;
	_localPosition4D = localTransform4D.Origin;
	// Will call `UpdatedTransform4D()`.
	SetLocalBasis4D(localTransform4D.Basis);
}

void UScene4DComponent::SetGlobalBasis4D(const FMatrix& InBasis)
{
	const UScene4DComponent* parent4D = GetParent4DComponent();
	if (parent4D == nullptr)
	{
		// Not attached to anything, so the local transform is the global transform.
		SetLocalBasis4D(InBasis); // Will call `UpdatedTransform4D()`.
		return;
	}
	// Will call `UpdatedTransform4D()`.
	SetLocalBasis4D(UBasis4D::Basis4D_Compose(UBasis4D::Basis4D_Inverse(parent4D->GetGlobalBasis4D()), InBasis));
}

// Global transform components.

FVector4 UScene4DComponent::GetGlobalPosition4D() const
{
	return GetGlobalTransform4D().Origin;
}

void UScene4DComponent::SetGlobalPosition4D(const FVector4& InPosition)
{
	FTransform4D globalTransform = GetGlobalTransform4D();
	globalTransform.Origin = InPosition;
	SetGlobalTransform4D(globalTransform);
}

FEuler4D UScene4DComponent::GetGlobalRotationRadians4D() const
{
	return FEuler4D::RadiansFromBasis(GetGlobalBasis4D());
}

void UScene4DComponent::SetGlobalRotationRadians4D(const FEuler4D& InRotationEulerRadians)
{
	const FVector4 globalScale = UBasis4D::Basis4D_GetScale(GetGlobalBasis4D());
	SetGlobalBasis4D(UBasis4D::Basis4D_ScaledLocal(InRotationEulerRadians.RadiansToBasis(), globalScale));
}

FEuler4D UScene4DComponent::GetGlobalRotationDegrees4D() const
{
	return FEuler4D::RadiansFromBasis(GetGlobalBasis4D()).RadToDeg();
}

void UScene4DComponent::SetGlobalRotationDegrees4D(const FEuler4D& InRotationEulerDegrees)
{
	const FVector4 globalScale = UBasis4D::Basis4D_GetScale(GetGlobalBasis4D());
	SetGlobalBasis4D(UBasis4D::Basis4D_ScaledLocal(InRotationEulerDegrees.DegToRad().RadiansToBasis(), globalScale));
}

FVector4 UScene4DComponent::GetGlobalScale4D() const
{
	return UBasis4D::Basis4D_GetScale(GetGlobalBasis4D());
}

void UScene4DComponent::SetGlobalScale4D(const FVector4& InScale)
{
	const FMatrix globalBasis = GetGlobalBasis4D();
	SetGlobalBasis4D(UBasis4D::Basis4D_WithScale(globalBasis, InScale));
}

double UScene4DComponent::GetGlobalUniformScale4D() const
{
	return UBasis4D::Basis4D_GetUniformScale(GetGlobalBasis4D());
}

void UScene4DComponent::SetGlobalUniformScale4D(const double InUniformScale)
{
	FMatrix globalBasis = GetGlobalBasis4D();
	SetGlobalBasis4D(UBasis4D::Basis4D_WithUniformScale(globalBasis, InUniformScale));
}

bool UScene4DComponent::IsVisibleInTree4D() const
{
	if (!IsVisible())
	{
		return false;
	}
	USceneComponent* parent = GetAttachParent();
	if (parent == nullptr)
	{
		return true;
	}
	UScene4DComponent* parent4D = Cast<UScene4DComponent>(parent);
	if (parent4D == nullptr)
	{
		return true;
	}
	return parent4D->IsVisibleInTree4D();
}

void UScene4DComponent::PropagateVisibilityChanged4D(bool bNewVisibility)
{
	OnVisibilityChanged4D.Broadcast(bNewVisibility);
	TArray<USceneComponent*> directChildren;
	GetChildrenComponents(false, directChildren);
	for (USceneComponent* directChild : directChildren)
	{
		UScene4DComponent* directChild4D = Cast<UScene4DComponent>(directChild);
		// A visiblility change here will only affect visible children, because
		// invisible children are invisible regardless of the parent's visibility.
		if (directChild4D != nullptr && directChild4D->IsVisible())
		{
			directChild4D->PropagateVisibilityChanged4D(bNewVisibility);
		}
	}
}

void UScene4DComponent::SetVisible4D(bool bNewVisibility)
{
	if (IsVisible() != bNewVisibility)
	{
		SetVisibility(bNewVisibility);
		PropagateVisibilityChanged4D(bNewVisibility);
	}
}

void UScene4DComponent::OnRegister()
{
	Super::OnRegister();
	// We need to have correct 3D world transform immediately after spawning
	// in order to fix the transform in the Blueprint editor viewport.
	UpdatedTransform4D();
}

void UScene4DComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	OnAttachmentHierarchyChanged4D();
}

void UScene4DComponent::OnAttachmentHierarchyChanged4D()
{
	const TArray<TObjectPtr<USceneComponent>>& children = GetAttachChildren();
	for (USceneComponent* child : children)
	{
		if (UScene4DComponent* child4D = Cast<UScene4DComponent>(child))
		{
			child4D->OnAttachmentHierarchyChanged4D();
		}
	}
}

// Rect bounds.

FRect4D UScene4DComponent::GetRectBoundsLocal4D(const FTransform4D& InToTarget) const
{
	// `InToTarget` transforms from the local space into the target space.
	// NOTE: If the target space IS the local space, `InToTarget` should be the identity transform.
	return FRect4D(); // Override in subclasses to provide actual bounds.
}

FRect4D UScene4DComponent::GetRectBoundsGlobal4D(const FTransform4D& InToTarget) const
{
	// `InToTarget` transforms from the global space into the target space.
	// Therefore, we compose it with the global transform to get local to target.
	// NOTE: If the target space IS the global space, `InToTarget` should be the identity transform.
	return GetRectBoundsLocal4D(InToTarget.Compose(GetGlobalTransform4D()));
}

FRect4D UScene4DComponent::GetRectBoundsGlobalRecursive4D(const FTransform4D& InToTarget) const
{
	// `InToTarget` transforms from the global space into the target space.
	// NOTE: If the target space IS the global space, `InToTarget` should be the identity transform.
	FRect4D bounds = GetRectBoundsGlobal4D(InToTarget);
	const TArray<TObjectPtr<USceneComponent>>& children = GetAttachChildren();
	for (USceneComponent* child : children)
	{
		UScene4DComponent* child4D = Cast<UScene4DComponent>(child);
		if (child4D != nullptr)
		{
			bounds = bounds.Merge(child4D->GetRectBoundsGlobalRecursive4D(InToTarget));
		}
	}
	return bounds;
}

#if WITH_EDITOR
void UScene4DComponent::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!_bSyncingFromEditor && InPropertyChangedEvent.Property != nullptr)
	{
		const FName name = InPropertyChangedEvent.GetMemberPropertyName();
		PostEditHandlePropertyName(name);
	}
	// CRITICAL: Super MUST be called AFTER our handling in order to get correct inspector behavior.
	Super::PostEditChangeProperty(InPropertyChangedEvent);
}

void UScene4DComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& InPropertyChangedChainEvent)
{
	if (!_bSyncingFromEditor)
	{
		FProperty* prop = nullptr;
		for (TDoubleLinkedList<FProperty*>::TDoubleLinkedListNode* node = InPropertyChangedChainEvent.PropertyChain.GetTail(); node != nullptr; node = node->GetPrevNode())
		{
			FProperty* p = node->GetValue();
			if (p == nullptr)
			{
				continue;
			}
			const UStruct* owner = p->GetOwnerStruct();
			if (owner == UScene4DComponent::StaticClass() || (owner != nullptr && owner->IsChildOf(UScene4DComponent::StaticClass())))
			{
				prop = p;
				break;
			}
		}
		if (prop != nullptr)
		{
			const FName name = prop->GetFName();
			PostEditHandlePropertyName(name);
		}
	}
	// CRITICAL: Super MUST be called AFTER our handling in order to get correct inspector behavior.
	Super::PostEditChangeChainProperty(InPropertyChangedChainEvent);
}

void UScene4DComponent::PostEditUndo()
{
	if (!_bSyncingFromEditor)
	{
		_bSyncingFromEditor = true;
		// Undo in the editor will restore the old basis, but the rotation and scale properties
		// will be stale, so we need to update them to match the restored basis.
		_localRotationDegrees4D = FEuler4D::RadiansFromBasis(_localBasis4D).RadToDeg();
		_localScale4D = UBasis4D::Basis4D_GetScale(_localBasis4D);
		_localUniformScale4D = UBasis4D::Basis4D_GetUniformScale(_localBasis4D);
		UpdatedTransform4D();
		// Some undo/redo paths restore transform fields but don't immediately refresh
		// render proxies for related components in the same actor. Force a render refresh
		// so 4D mesh visuals match the restored transform in the editor viewport.
		if (AActor* owner = GetOwner())
		{
			owner->MarkComponentsRenderStateDirty();
		}
		_bSyncingFromEditor = false;
	}
	// CRITICAL: Super MUST be called AFTER our handling in order to get correct inspector behavior.
	Super::PostEditUndo();
}

void UScene4DComponent::PostEditHandlePropertyName(const FName& InPropertyName)
{
	_bSyncingFromEditor = true;
	// Handle any co-dependent property changes based on the property name.
	// Sync up the basis with decomposed rotation and scale properties, and vice versa.
	if (InPropertyName == GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localBasis4D))
	{
		_localRotationDegrees4D = FEuler4D::RadiansFromBasis(_localBasis4D).RadToDeg();
		_localScale4D = UBasis4D::Basis4D_GetScale(_localBasis4D);
		_localUniformScale4D = UBasis4D::Basis4D_GetUniformScale(_localBasis4D);
	}
	else if (InPropertyName == GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localRotationDegrees4D))
	{
		_localBasis4D = _localRotationDegrees4D.DegToRad().RadiansToBasis();
		_localBasis4D = UBasis4D::Basis4D_ScaledLocal(_localBasis4D, _localScale4D);
	}
	else if (InPropertyName == GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localScale4D))
	{
		_localBasis4D = UBasis4D::Basis4D_WithScale(_localBasis4D, _localScale4D);
		_localUniformScale4D = UBasis4D::Basis4D_GetUniformScale(_localBasis4D);
	}
	else if (InPropertyName == GET_MEMBER_NAME_CHECKED(UScene4DComponent, _localUniformScale4D))
	{
		_localBasis4D = UBasis4D::Basis4D_WithUniformScale(_localBasis4D, _localUniformScale4D);
		_localScale4D = UBasis4D::Basis4D_GetScale(_localBasis4D);
	}
	UpdatedTransform4D();
	_bSyncingFromEditor = false;
}
#endif // WITH_EDITOR
