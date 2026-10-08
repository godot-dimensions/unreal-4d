#include "EditorTransformProxy4D.h"

#include "../../Unreal4DEditorMode.h"

#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "ScopedTransaction.h"

FTransform4D UEditorTransformProxy4D::LatestProxyTransformStatic = FTransform4D::Identity();
bool UEditorTransformProxy4D::bIsTransformingAndIgnoringObjectTransformations = false;

bool UEditorTransformProxy4D::IsShiftDownInEditor()
{
	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}
	const FModifierKeysState modifiers = FSlateApplication::Get().GetModifierKeys();
	return modifiers.IsShiftDown();
}

void UEditorTransformProxy4D::ResetFollowState()
{
	_bCameraFollowShiftWasDown = false;
	_cameraFollowOriginalPosition4D = FVector4::Zero();
	_cameraFollowRelativeOffset4D = FVector4::Zero();
}

FUnreal4DEditorMode* UEditorTransformProxy4D::GetEditorMode() const
{
	if (_modeTools == nullptr)
	{
		return nullptr;
	}
	return static_cast<FUnreal4DEditorMode*>(_modeTools->GetActiveMode(FUnreal4DEditorMode::ModeID));
}

FVector4 UEditorTransformProxy4D::GetEditorCameraPosition4D() const
{
	if (_viewportClient == nullptr)
	{
		ensure(false);
		return FVector4::Zero();
	}
	FUnreal4DEditorMode* editorMode = GetEditorMode();
	if (editorMode == nullptr)
	{
		ensure(false);
		return FVector4::Zero();
	}
	return editorMode->GetEditorCameraPosition4D(_viewportClient);
}

void UEditorTransformProxy4D::SetEditorCameraPosition4D(const FVector4& InNewPosition) const
{
	if (_viewportClient == nullptr)
	{
		return;
	}
	FUnreal4DEditorMode* editorMode = GetEditorMode();
	if (editorMode == nullptr)
	{
		return;
	}
	editorMode->SetEditorCameraPosition4D(_viewportClient, InNewPosition);
}

void UEditorTransformProxy4D::SetCoordSystem(ECoordSystem InCoordSystem)
{
	_coordSystem = InCoordSystem;
	UpdateProxyTransformFromObjects();
}

void UEditorTransformProxy4D::SetEditorContext(FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient)
{
	_modeTools = InModeTools;
	_viewportClient = InViewportClient;
}

bool UEditorTransformProxy4D::DuplicateSelectedObjects()
{
	// Only attempt to duplicate if we are currently transforming and have selected objects.
	if (!bIsTransformingAndIgnoringObjectTransformations || _objects4D.Num() == 0)
	{
		return false;
	}
	// Duplicate the actors that own the selected components.
	TSet<AActor*> processedActors;
	for (UScene4DComponent* obj4D : _objects4D)
	{
		if (obj4D == nullptr || !IsValid(obj4D) || !obj4D->IsValidLowLevel())
		{
			continue;
		}
		AActor* sourceActor = obj4D->GetOwner();
		if (sourceActor == nullptr || !IsValid(sourceActor))
		{
			continue;
		}
		if (processedActors.Contains(sourceActor))
		{
			continue;
		}
		processedActors.Add(sourceActor);
		ULevel* level = sourceActor->GetLevel();
		if (level == nullptr)
		{
			continue;
		}
		// Mark the level's actor list as modified so undo can remove the new actor.
		level->Modify();
		// Copy the entire actor, components, and properties (but do NOT duplicate those properties, like meshes).
		FObjectDuplicationParameters dupParams = FObjectDuplicationParameters(sourceActor, level);
		AActor* newActor = Cast<AActor>(StaticDuplicateObjectEx(dupParams));
		if (newActor == nullptr || !IsValid(newActor))
		{
			continue;
		}
		// Ensure the new actor participates in undo transactions so undo can remove it.
		newActor->SetFlags(RF_Transactional);
		newActor->Modify();
		// Add to the level's actor list so it is tracked by the world.
		level->Actors.Add(newActor);
		// Initialize actor-specific duplicate state (e.g. clears shared transient handles).
		newActor->PostDuplicate(false);
		// Register all components with the world so they appear in the viewport.
		newActor->ReregisterAllComponents();
		// Notify editor systems (World Outliner, etc.) that a new actor has been added.
		GEditor->BroadcastLevelActorAdded(newActor);
	}
	return true;
}

void UEditorTransformProxy4D::Select4DObjects(const TArray<UScene4DComponent*>& InObjects4D)
{
	// Unbind from the old objects.
	for (UScene4DComponent* obj4D : _objects4D)
	{
		if (obj4D != nullptr && IsValid(obj4D) && obj4D->IsValidLowLevel())
		{
			obj4D->OnTransform4DChangedDelegate.RemoveAll(this);
		}
	}
	_objects4D = InObjects4D;
	UpdateProxyTransformFromObjects();
	// Bind to the new objects.
	for (UScene4DComponent* obj4D : _objects4D)
	{
		// Make sure we are only binding to valid objects.
		if (!ensure(obj4D != nullptr && IsValid(obj4D) && obj4D->IsValidLowLevel()))
		{
			continue;
		}
		obj4D->OnTransform4DChangedDelegate.AddDynamic(this, &UEditorTransformProxy4D::OnComponent4DTransformChanged);
	}
}

void UEditorTransformProxy4D::OnComponent4DTransformChanged(UScene4DComponent* InChangedComponent)
{
	UpdateProxyTransformFromObjects();
}

FTransform4D UEditorTransformProxy4D::GetParentGlobalTransform(const UScene4DComponent* InObj4D)
{
	if (InObj4D == nullptr || !IsValid(InObj4D) || !InObj4D->IsValidLowLevel())
	{
		return FTransform4D::Identity();
	}
	UScene4DComponent* parentObj4D = Cast<UScene4DComponent>(InObj4D->GetAttachParent());
	if (parentObj4D == nullptr || !IsValid(parentObj4D) || !parentObj4D->IsValidLowLevel())
	{
		return FTransform4D::Identity();
	}
	return parentObj4D->GetGlobalTransform4D();
}

void UEditorTransformProxy4D::UpdateProxyTransformFromObjects()
{
	if (bIsTransformingAndIgnoringObjectTransformations)
	{
		return;
	}
	// First, validate the objects.
	TArray<UScene4DComponent*> validObjects4D;
	for (UScene4DComponent* obj4D : _objects4D)
	{
		if (obj4D != nullptr && IsValid(obj4D) && obj4D->IsValidLowLevel())
		{
			validObjects4D.Add(obj4D);
		}
	}
	_objects4D = validObjects4D;
	// Recalculate the transform data for the valid objects.
	_oldObjectGlobalTransforms4D.Reset();
	FTransform4D sumTransform4D = FTransform4D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	int transformCount = 0;
	for (UScene4DComponent* obj4D : _objects4D)
	{
		FTransform4D objGlobalTransform4D = obj4D->GetGlobalTransform4D();
		_oldObjectGlobalTransforms4D.Add(objGlobalTransform4D);
		sumTransform4D = sumTransform4D + objGlobalTransform4D;
		transformCount++;
	}
	_oldProxyTransform4D = FTransform4D::Identity();
	if (transformCount > 0)
	{
		_oldProxyTransform4D.Origin = sumTransform4D.Origin / (double)transformCount;
		switch (_coordSystem)
		{
			case COORD_World: // Move and rotate objects relative to the cardinal world axes.
			{
				// Do nothing, the basis is already identity which is what we want for world space.
			} break;
			case COORD_Local: // Move and rotate objects relative to the object's local axes.
			{
				_oldProxyTransform4D.Basis = UBasis4D::Basis4D_ScaledUniform(sumTransform4D.Basis, 1.0 / (double)transformCount);
				_oldProxyTransform4D.Orthonormalize();
			} break;
			case COORD_Parent: // Move and rotate objects relative to the parent's axes.
			case COORD_Explicit: // Move objects relative to the parent's axes and rotate them using gimbal axes.
			{
				FTransform4D parentTransform4D = GetParentGlobalTransform(_objects4D[0]);
				_oldProxyTransform4D.Basis = parentTransform4D.Basis;
				_oldProxyTransform4D.Orthonormalize();
			} break;
			case COORD_None:
			case COORD_Max:
			{
				UE_LOG(LogUnreal4DEditor, Warning, TEXT("UpdateProxyTransformFromObjects: Unhandled coordinate system enum value %d."), (int)_coordSystem);
			} break;
		}
	}
	_currentProxyTransform4D = _oldProxyTransform4D;
	LatestProxyTransformStatic = _currentProxyTransform4D;
	On4DTransformProxyChanged.Broadcast(_currentProxyTransform4D);
}

FVector4 UEditorTransformProxy4D::GetShiftSlideOffset()
{
	if (!_bCameraFollowShiftWasDown)
	{
		return FVector4::Zero();
	}
	FVector4 editorCameraPosition4D = GetEditorCameraPosition4D();
	return editorCameraPosition4D - _cameraFollowOriginalPosition4D;
}

void UEditorTransformProxy4D::ApplyTransformChange(const FTransform4D& InTransformChange, const bool bInUseKeepMode)
{
	// Capture the camera position relative to the gizmo origin on Shift key-down.
	const bool bShiftDown = IsShiftDownInEditor();
	if (bShiftDown && !_bCameraFollowShiftWasDown)
	{
		_cameraFollowOriginalPosition4D = GetEditorCameraPosition4D();
		_cameraFollowRelativeOffset4D = _cameraFollowOriginalPosition4D - _currentProxyTransform4D.Origin;
	}
	_bCameraFollowShiftWasDown = bShiftDown;
	// Check if the transform change is valid before applying it.
	const double determinant = InTransformChange.Basis.Determinant();
	if (FMath::IsNearlyZero(determinant, 1e-20) || !FMath::IsFinite(determinant))
	{
		UE_LOG(LogUnreal4DEditor, Warning, TEXT("ApplyTransformChange: Aborting transform change with zero or near-zero determinant (%f)."), determinant);
		ensure(false);
		return;
	}
	const int32 objectCount = _objects4D.Num();
	if (objectCount != _oldObjectGlobalTransforms4D.Num())
	{
		UE_LOG(LogUnreal4DEditor, Warning, TEXT("ApplyTransformChange: Transform cache size mismatch: %d != %d"), objectCount, _oldObjectGlobalTransforms4D.Num());
		ensure(false);
		return;
	}
	// Apply the transform change to the current proxy transform and all of the selected objects.
	_currentProxyTransform4D = InTransformChange.Compose(_oldProxyTransform4D);
	for (int i = 0; i < objectCount; i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (obj4D == nullptr || !IsValid(obj4D) || !obj4D->IsValidLowLevel())
		{
			continue;
		}
		FTransform4D newGlobalTransform4D = InTransformChange.Compose(_oldObjectGlobalTransforms4D[i]);
		obj4D->SetGlobalTransform4D(newGlobalTransform4D);
		if (SnapSettings->bSnapFinalValues) {
			obj4D->SetLocalTransform4D(SnapSettings->SnapSingleTransform(obj4D->GetLocalTransform4D()));
		}
		// Note: The keep mode takes priority over snapping, so we apply it after snapping.
		// In most cases these won't conflict, but in an edge case where they do, keep mode wins.
		if (bInUseKeepMode) {
			switch (_keepMode)
			{
				case EEditorTransform4DKeepMode::Freeform:
					// No additional adjustments needed.
					break;
				case EEditorTransform4DKeepMode::Orthogonal:
					obj4D->SetLocalTransform4D(obj4D->GetLocalTransform4D().Orthogonalized());
					break;
				case EEditorTransform4DKeepMode::ConformalChangesOnly:
					// Do nothing in this case, we only want the changes to be conformal, not the resulting transform.
					break;
				case EEditorTransform4DKeepMode::ConformalChangesAndResults:
					obj4D->SetLocalTransform4D(obj4D->GetLocalTransform4D().Conformalized());
					break;
				case EEditorTransform4DKeepMode::Orthonormal:
					obj4D->SetLocalTransform4D(obj4D->GetLocalTransform4D().Orthonormalized());
					break;
			}
		}
	}
	if (bShiftDown)
	{
		const FVector4 newCameraPosition = _currentProxyTransform4D.Origin + _cameraFollowRelativeOffset4D;
		SetEditorCameraPosition4D(newCameraPosition);
	}
	On4DTransformProxyChanged.Broadcast(_currentProxyTransform4D);
}

void UEditorTransformProxy4D::BeginTransformations()
{
	UpdateProxyTransformFromObjects(); // Last update...
	if (!_activeTransformTransaction.IsValid() && !GIsTransacting)
	{
		_activeTransformTransaction = MakeUnique<FScopedTransaction>(NSLOCTEXT("EditorTransformProxy4D", "TransformWithDuplicate", "Transform 4D"));
	}
	bIsTransformingAndIgnoringObjectTransformations = true; // .. then ignore everything until the transformation is done.
	ResetFollowState();
	// Inform the snap settings of the starting distance between the gizmo and the editor camera.
	FVector4 editorCameraPosition4D = GetEditorCameraPosition4D();
	const FVector4 editorCameraOffset = _oldProxyTransform4D.Origin - editorCameraPosition4D;
	SnapSettings->CameraDistance = editorCameraOffset.Size();
}

void UEditorTransformProxy4D::CancelTransformations()
{
	for (int i = 0; i < _objects4D.Num(); i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (obj4D == nullptr || !IsValid(obj4D) || !obj4D->IsValidLowLevel())
		{
			continue;
		}
		obj4D->SetGlobalTransform4D(_oldObjectGlobalTransforms4D[i]);
	}
	// Begin accepting transform changes again and update the proxy transform to match the new state of the objects.
	ResetFollowState();
	bIsTransformingAndIgnoringObjectTransformations = false;
	_activeTransformTransaction.Reset();
	UpdateProxyTransformFromObjects();
}

void UEditorTransformProxy4D::CommitTransformations()
{
	const int32 objectCount = _objects4D.Num();
	if (objectCount == 0 || objectCount != _oldObjectGlobalTransforms4D.Num())
	{
		bIsTransformingAndIgnoringObjectTransformations = false;
		_activeTransformTransaction.Reset();
		UpdateProxyTransformFromObjects();
		return;
	}
	// Gather the new transforms for all objects so that we don't lose this when modifying objects.
	TArray<FTransform4D> newObjectTransforms4D;
	newObjectTransforms4D.Reserve(objectCount);
	for (int32 i = 0; i < objectCount; i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (!IsValid(obj4D))
		{
			newObjectTransforms4D.Add(FTransform4D::Identity());
			continue;
		}
		newObjectTransforms4D.Add(obj4D->GetGlobalTransform4D());
	}
	// Restore old transforms so Modify() captures the pre-change state.
	for (int32 i = 0; i < objectCount; i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (!IsValid(obj4D))
		{
			continue;
		}
		obj4D->SetGlobalTransform4D(_oldObjectGlobalTransforms4D[i]);
	}
	// Record each of the components and owning actors into the current transaction (ours from BeginTransformations).
	for (int32 i = 0; i < objectCount; i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (obj4D == nullptr || !IsValid(obj4D) || !obj4D->IsValidLowLevel())
		{
			continue;
		}
		if (AActor* owner = obj4D->GetOwner())
		{
			owner->Modify();
			//TArray<UActorComponent*> ownerComponents;
			//owner->GetComponents(ownerComponents);
			//for (UActorComponent* ownerComponent : ownerComponents)
			//{
			//	if (ownerComponent != nullptr && IsValid(ownerComponent) && ownerComponent->IsValidLowLevel())
			//	{
			//		ownerComponent->Modify();
			//	}
			//}
		}
		obj4D->Modify();
		// In the Blueprint SCS "Simple Construction Script" viewport, some changes may be
		// made but not persist because the SCS viewport edits "preview instances".
		// In order to make changes persist, we need to apply the changes to the "archetype".
		// However, we must not do this in the Level editor viewport, or changes will be lost/reverted.
		UWorld* world = obj4D->GetWorld();
		if (world != nullptr && world->WorldType == EWorldType::EditorPreview)
		{
			UScene4DComponent* obj4DTemplateArchetype = Cast<UScene4DComponent>(obj4D->GetArchetype());
			if (obj4DTemplateArchetype != nullptr && obj4DTemplateArchetype != obj4D)
			{
				obj4DTemplateArchetype->Modify();
			}
		}
	}
	// Now that the old state of the objects have been recorded inside of the
	// transaction via `Modify()`, we can restore the new transforms to them.
	for (int32 i = 0; i < objectCount; i++)
	{
		UScene4DComponent* obj4D = _objects4D[i];
		if (obj4D == nullptr || !IsValid(obj4D) || !obj4D->IsValidLowLevel())
		{
			continue;
		}
		obj4D->SetGlobalTransform4D(newObjectTransforms4D[i]);
		// Set the new local transform on the "archetype" for Blueprint SCS persistence.
		// However, we must not do this in the Level editor viewport, or changes will be lost/reverted.
		UWorld* world = obj4D->GetWorld();
		if (world != nullptr && world->WorldType == EWorldType::EditorPreview)
		{
			UScene4DComponent* obj4DTemplateArchetype = Cast<UScene4DComponent>(obj4D->GetArchetype());
			if (obj4DTemplateArchetype != nullptr && obj4DTemplateArchetype != obj4D)
			{
				obj4DTemplateArchetype->SetLocalTransform4D(obj4D->GetLocalTransform4D());
				obj4DTemplateArchetype->PostEditChange();
			}
		}
	}
	// Begin accepting transform changes again and update the proxy transform to match the new state of the objects.
	ResetFollowState();
	bIsTransformingAndIgnoringObjectTransformations = false;
	_activeTransformTransaction.Reset();
	UpdateProxyTransformFromObjects();
}
