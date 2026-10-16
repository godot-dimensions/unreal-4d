#include "Unreal4DEditorMode.h"

#include "../Unreal4DCore/Render/RenderingServer4D.h"
#include "../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "Unreal4DEditorCommands.h"
#include "Viewport/TransformGizmo/EditorTransformGizmo4D.h"
#include "Viewport/TransformGizmo/EditorTransformGizmo4DFactory.h"
#include "Viewport/TransformGizmo/EditorTransformProxy4D.h"

// Globals.
#include "Editor/UnrealEdEngine.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "EditorViewportCommands.h"
#include "LevelEditorViewport.h"
#include "Settings/EditorStyleSettings.h"
#include "UnrealEdGlobals.h"
#include "EngineUtils.h"

// Unique editor mode ID, used to activate and identify this editor mode within Unreal.
const FEditorModeID FUnreal4DEditorMode::ModeID = TEXT("EM_Unreal4DEditorMode");

FEditorViewportCameraTransform4D& FUnreal4DEditorMode::GetOrAddViewportCameraTransform(FEditorViewportClient* InViewportClient)
{
	return _viewportCameraTransforms.FindOrAdd(InViewportClient);
}

bool FUnreal4DEditorMode::DoesKeyMatchCommand(const FKey& InKey, FViewport* InViewport, const TSharedPtr<FUICommandInfo>& InCommand)
{
	if (!InCommand.IsValid())
	{
		return false;
	}
	auto CheckChord = [&](EMultipleKeyBindingIndex ChordIndex) -> bool
	{
		const FInputChord Chord = FUnreal4DEditorCommands::GetEffectiveActiveChord(InCommand, ChordIndex);
		if (!Chord.Key.IsValid() || Chord.Key != InKey)
		{
			return false;
		}
		const bool bCtrl = InViewport->KeyState(EKeys::LeftControl) || InViewport->KeyState(EKeys::RightControl);
		const bool bShift = InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift);
		const bool bAlt = InViewport->KeyState(EKeys::LeftAlt) || InViewport->KeyState(EKeys::RightAlt);
		const bool bCmd = InViewport->KeyState(EKeys::LeftCommand) || InViewport->KeyState(EKeys::RightCommand);
		return Chord.bCtrl == bCtrl && Chord.bShift == bShift && Chord.bAlt == bAlt && Chord.bCmd == bCmd;
	};
	return CheckChord(EMultipleKeyBindingIndex::Primary) || CheckChord(EMultipleKeyBindingIndex::Secondary);
}

bool FUnreal4DEditorMode::IsCommandCurrentlyDown(FViewport* InViewport, const TSharedPtr<FUICommandInfo>& InCommand)
{
	if (InViewport == nullptr || !InCommand.IsValid())
	{
		return false;
	}
	auto CheckChord = [&](EMultipleKeyBindingIndex ChordIndex) -> bool
	{
		const FInputChord Chord = FUnreal4DEditorCommands::GetEffectiveActiveChord(InCommand, ChordIndex);
		if (!Chord.Key.IsValid())
		{
			return false;
		}
		if (!InViewport->KeyState(Chord.Key))
		{
			return false;
		}
		const bool bCtrl = InViewport->KeyState(EKeys::LeftControl) || InViewport->KeyState(EKeys::RightControl);
		const bool bShift = InViewport->KeyState(EKeys::LeftShift) || InViewport->KeyState(EKeys::RightShift);
		const bool bAlt = InViewport->KeyState(EKeys::LeftAlt) || InViewport->KeyState(EKeys::RightAlt);
		const bool bCmd = InViewport->KeyState(EKeys::LeftCommand) || InViewport->KeyState(EKeys::RightCommand);
		return Chord.bCtrl == bCtrl && Chord.bShift == bShift && Chord.bAlt == bAlt && Chord.bCmd == bCmd;
	};
	return CheckChord(EMultipleKeyBindingIndex::Primary) || CheckChord(EMultipleKeyBindingIndex::Secondary);
}

FLinearColor FUnreal4DEditorMode::GetAxisColor(const EAxis4D InAxis) {
	const UEditorStyleSettings* settings = GetDefault<UEditorStyleSettings>();
	if (settings != nullptr) {
		switch (InAxis) {
			case EAxis4D::X : return settings->XAxisColor;
			case EAxis4D::Y : return settings->YAxisColor;
			case EAxis4D::Z : return settings->ZAxisColor;
			// There is no W axis colour setting. Just fall through to the backup fixed colour.
		}
	}
	return UVectorFuncs4D::Vector4_AxisColor(InAxis);
}

void FUnreal4DEditorMode::Exit()
{
	_transformGizmoManager.Shutdown(*GetModeManager());
	UEditorTransformGizmo4DFactory::RegisterViewportClientForModeTools(GetModeManager(), nullptr);
	if (GEditor != nullptr)
	{
		// Only visit live viewport clients; a previously registered viewport may have been closed.
		for (FLevelEditorViewportClient* viewportClient : GEditor->GetLevelViewportClients())
		{
			viewportClient->OnDropElementTransformUpdated().RemoveAll(this);
		}
	}
	_levelViewportClientsWithRegisteredSignals.Empty();
	FEdMode::Exit();
}

void FUnreal4DEditorMode::ActorSelectionChangeNotify()
{
	FEdMode::ActorSelectionChangeNotify();
	_transformGizmoManager.InvalidateSelection();
}

void FUnreal4DEditorMode::Tick(FEditorViewportClient* InViewportClient, float InDeltaTime)
{
	FEdMode::Tick(InViewportClient, InDeltaTime);
	_transformGizmoManager.Update(*GetModeManager());
	// Register for drag-and-drop transform updates if not already registered.
	// We need to check this on each editor mode tick because it is possible
	// for viewport clients to be added or removed (ex: split viewports).
	if (!_levelViewportClientsWithRegisteredSignals.Contains(InViewportClient))
	{
		// Only register with FLevelEditorViewportClient, not other viewport types (such as FSCSEditorViewportClient).
		// Since this isn't a UObject, a direct cast doesn't work, so we need to determine this based on the world type.
		UWorld* viewportWorld = InViewportClient->GetWorld();
		if (viewportWorld && !viewportWorld->IsGameWorld() && viewportWorld->WorldType == EWorldType::Editor)
		{
			FLevelEditorViewportClient* levelViewportClient = static_cast<FLevelEditorViewportClient*>(InViewportClient);
			levelViewportClient->OnDropElementTransformUpdated().AddRaw(this, &FUnreal4DEditorMode::OnDropElementTransformUpdated);
			_levelViewportClientsWithRegisteredSignals.Add(levelViewportClient);
		}
	}
	// Re-evaluate which viewport navigation chords conflict with our 4D keybindings.
	FUnreal4DEditorCommands::UpdateViewportNavChordBlocking();
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	// Update the 3D camera transform to account for any changes the 3D editor made to it.
	const FVector viewportCameraLocation = InViewportClient->GetViewLocation();
	FRotator viewportCameraRotation = InViewportClient->GetViewRotation();
	viewCam.PitchDegrees = viewportCameraRotation.Pitch;
	// Get the ground part of the transform by zeroing out the pitch (and roll, shouldn't happen but just in case).
	viewportCameraRotation.Pitch = 0.0;
	viewportCameraRotation.Roll = 0.0;
	viewCam.GroundTransform3D = FTransform(viewportCameraRotation, viewportCameraLocation);
	const FTransform4D cameraGroundTransform3Dbutin4D = FTransform4D::From3D(viewCam.GroundTransform3D);
	viewCam.GroundTransform4D = viewCam.ParentOfGroundTransform3Dtomakeit4D.Compose(cameraGroundTransform3Dbutin4D);
	// Apply per-tick motion in W axis from Q and E keys, if any.
	const double localWMovement = GetLocalWAxisMovement();
	if (!FMath::IsNearlyZero(localWMovement))
	{
		// Use viewport client's camera speed for W-axis movement to match 3D camera feel.
		const double cameraSpeed = WAxisMoveSpeed * InViewportClient->GetCameraSpeed();
		viewCam.GroundTransform4D.TranslateLocal(FVector4(0.0, 0.0, 0.0, localWMovement * InDeltaTime * cameraSpeed));
		viewCam.ParentOfGroundTransform3Dtomakeit4D = cameraGroundTransform3Dbutin4D.TransformTo(viewCam.GroundTransform4D);
	}
	// Update the rendering server's override camera transform
	// so that the editor camera can control the 4D view.
	UpdateRenderingServerWithEditorCameraTransform(InViewportClient);
	// When the user presses F, 2 things happen: the 4D focus which we handle,
	// and the 3D focus which Unreal handles. The 4D focus brings the camera's
	// slice into intersection with the object, but if the object was outside
	// of the slice before, Unreal's 3D focus sees zero-size bounds, which moves
	// the camera very close to the object, which becomes the inside after slicing.
	// To fix this, we defer additional 3D focuses for a few ticks after a 4D focus,
	// which lets Unreal's 3D focus properly handle the now-in-slice object bounds.
	if (_pendingRefocus3DFrames > 0)
	{
		GUnrealEd->Exec(InViewportClient->GetWorld(), TEXT("CAMERA ALIGN ACTIVEVIEWPORTONLY"));
		_pendingRefocus3DFrames--;
	}
}

void FUnreal4DEditorMode::UpdateRenderingServerWithEditorCameraTransform(FEditorViewportClient* InViewportClient)
{
	UWorld* world = InViewportClient->GetWorld();
	if (world == nullptr)
	{
		return;
	}
	URenderingServer4D* renderingServer4D = world->GetSubsystem<URenderingServer4D>();
	if (renderingServer4D == nullptr)
	{
		return;
	}
	// Note: For the purposes of the editor camera, we don't actually need to send the pitch to the rendering server.
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	renderingServer4D->SetOverrideCameraTransforms(viewCam.GroundTransform4D, viewCam.GroundTransform3D);
}

bool FUnreal4DEditorMode::InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey, EInputEvent InEvent)
{
	// We only want to handle pressed and released events, ignore Repeat, DoubleClick, and Axis events.
	if (InEvent != EInputEvent::IE_Pressed && InEvent != EInputEvent::IE_Released)
	{
		return FEdMode::InputKey(InViewportClient, InViewport, InKey, InEvent);
	}
	const bool bIsPressed = InEvent == EInputEvent::IE_Pressed;
	const FUnreal4DEditorCommands& commands = FUnreal4DEditorCommands::Get();
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	// Focus on selected objects (reuses Unreal's built-in focus command keybinding).
	// Unreal already handles the 3D part of this, so we just need to update
	// the 4D slice position to move the camera to the selected objects.
	if (DoesKeyMatchCommand(InKey, InViewport, FEditorViewportCommands::Get().FocusViewportToSelection))
	{
		if (bIsPressed && !bRightMouseDown)
		{
			// Using a static variable seems a bit unfortunate but the gizmo and editor mode don't share a common context...
			const FTransform4D gizmoTransform4D = UEditorTransformProxy4D::LatestProxyTransformStatic;
			const FVector4 camToGizmoDisplacement = gizmoTransform4D.Origin - viewCam.GroundTransform4D.Origin;
			const FVector4 camPerpDirection = UBasis4D::Basis4D_GetRow(viewCam.GroundTransform4D.Basis, 3);
			const FVector4 camToGizmoPerpOffset = UVectorFuncs4D::Vector4_Project(camToGizmoDisplacement, camPerpDirection);
			viewCam.GroundTransform4D.Origin += camToGizmoPerpOffset;
			const FTransform4D cameraGroundTransform3Dbutin4D = FTransform4D::From3D(viewCam.GroundTransform3D);
			viewCam.ParentOfGroundTransform3Dtomakeit4D = cameraGroundTransform3Dbutin4D.TransformTo(viewCam.GroundTransform4D);
			// Defer additional 3D focuses so Unreal re-frames with the now-valid bounds.
			_pendingRefocus3DFrames = 3;
		}
		// It doesn't really matter if we consume this or not. We want Unreal to handle the 3D focus,
		// but we are deferring it to happen a few ticks later anyway, so let's just consume.
		return true;
	}
	// Reset the 4D slice.
	else if (DoesKeyMatchCommand(InKey, InViewport, commands.Reset4DSlice))
	{
		if (bIsPressed)
		{
			viewCam.ParentOfGroundTransform3Dtomakeit4D = FTransform4D::Identity();
			const FTransform4D cameraGroundTransform3Dbutin4D = FTransform4D::From3D(viewCam.GroundTransform3D);
			viewCam.GroundTransform4D = cameraGroundTransform3Dbutin4D;
		}
		// Consume to prevent default 3D behavior.
		return true;
	}
	// Right mouse button tracking. This is not configurable because Unreal's
	// built-in right-click camera look is hardcoded (not a UI_COMMAND chord).
	else if (InKey == EKeys::RightMouseButton)
	{
		bRightMouseDown = bIsPressed;
	}
	else
	{
		if (DoesKeyMatchCommand(InKey, InViewport, commands.Rotate4DModifier))
		{
			bRotate4DModifierDown = bIsPressed;
		}
		else if (DoesKeyMatchCommand(InKey, InViewport, commands.WAxisPositive))
		{
			bWAxisPositiveDown = bIsPressed;
		}
		else if (DoesKeyMatchCommand(InKey, InViewport, commands.WAxisNegative))
		{
			bWAxisNegativeDown = bIsPressed;
		}
		else if ((InKey.IsSameResolvedKey(EKeys::LeftAlt) || InKey.IsSameResolvedKey(EKeys::RightAlt)) && bIsPressed)
		{
			// This uses hard-coded Alt keys instead of commands because Unreal's editor keyboard
			// shortcuts menu does not allow binding modifier keys by themselves, and the goal
			// is parity with the 3D gizmo, so rebinding doesn't need to be supported.
			const bool bHandledBy4DGizmo = UEditorTransformGizmo4D::DuplicateSelectedObjectsStatic();
			if (bHandledBy4DGizmo)
			{
				return true;
			}
			return FEdMode::InputKey(InViewportClient, InViewport, InKey, InEvent);
		}
		else
		{
			// Let the default mode handle other keys.
			return FEdMode::InputKey(InViewportClient, InViewport, InKey, InEvent);
		}
		// Consume configurable 4D keys when right mouse is held to prevent default 3D behavior.
		if (bRightMouseDown)
		{
			return true;
		}
	}
	// Let the default mode handle other keys or cases when right mouse is not held.
	return FEdMode::InputKey(InViewportClient, InViewport, InKey, InEvent);
}

bool FUnreal4DEditorMode::InputAxis(FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InControllerId, FKey InKey, float InDelta, float InDeltaTime)
{
	// Keep modifier state in sync with actual hardware key state so missed release events cannot latch it.
	// This fixes a problem that occurs when holding shift and dragging the gizmo,
	// then releasing left click before shift, which causes Unreal to never send
	// us the shift release event and thus we'd think left click is still held.
	bRotate4DModifierDown = IsCommandCurrentlyDown(InViewport, FUnreal4DEditorCommands::Get().Rotate4DModifier);
	// Handle mouse movement - consume it if we're in 4D mode to prevent default camera rotation.
	if (InKey == EKeys::MouseX || InKey == EKeys::MouseY)
	{
		if (ShouldMouseMovementRotateIn4D())
		{
			FMatrix rotationDelta;
			if (InKey == EKeys::MouseX)
			{
				rotationDelta = UBasis4D::Basis4D_FromWY(InDelta * 0.002);
			}
			else // InKey == EKeys::MouseY
			{
				rotationDelta = UBasis4D::Basis4D_FromXW(InDelta * 0.002);
			}
			RotateEditorCameraLocal4D(InViewportClient, rotationDelta);
			// Consume to block default 3D editor camera rotation.
			return true;
		}
	}
	// Let the default mode handle other axes or cases when
	// right mouse plus left or middle mouse is not held.
	return FEdMode::InputAxis(InViewportClient, InViewport, InControllerId, InKey, InDelta, InDeltaTime);
}

FTransform4D FUnreal4DEditorMode::GetFullEditorCameraTransform4D(FEditorViewportClient* InViewportClient)
{
	if (InViewportClient == nullptr)
	{
		return FTransform4D::Identity();
	}
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	const double pitchRadiansZX = viewCam.PitchDegrees * (-UNREAL4D_TAU / 360.0);
	return viewCam.GroundTransform4D.Compose(FTransform4D(UBasis4D::Basis4D_FromZX(pitchRadiansZX)));
}

FTransform4D FUnreal4DEditorMode::GetEditorCameraGroundTransform4D(FEditorViewportClient* InViewportClient)
{
	if (InViewportClient == nullptr)
	{
		return FTransform4D::Identity();
	}
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	return viewCam.GroundTransform4D;
}

FTransform FUnreal4DEditorMode::GetEditorCameraGroundTransform3D(FEditorViewportClient* InViewportClient)
{
	if (InViewportClient == nullptr)
	{
		return FTransform::Identity;
	}
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	return viewCam.GroundTransform3D;
}

FVector4 FUnreal4DEditorMode::GetEditorCameraPosition4D(FEditorViewportClient* InViewportClient)
{
	if (InViewportClient == nullptr)
	{
		return FVector4::Zero();
	}
	return GetOrAddViewportCameraTransform(InViewportClient).GroundTransform4D.Origin;
}

void FUnreal4DEditorMode::RotateEditorCameraLocal4D(FEditorViewportClient* InViewportClient, const FMatrix& InLocalRotationBasis4D)
{
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	const FMatrix newBasis = UBasis4D::Basis4D_Compose(viewCam.GroundTransform4D.Basis, InLocalRotationBasis4D);
	SetEditorCameraGroundBasis4D(InViewportClient, UBasis4D::Basis4D_Orthonormalized(newBasis));
}

void FUnreal4DEditorMode::SetEditorCameraGroundBasis4D(FEditorViewportClient* InViewportClient, const FMatrix& InNewGroundBasis)
{
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	viewCam.GroundTransform4D.Basis = InNewGroundBasis;
	// Update the 3D camera transform to account for any changes the 3D editor made to it.
	const FVector viewportCameraLocation = InViewportClient->GetViewLocation();
	FRotator viewportCameraRotation = InViewportClient->GetViewRotation();
	viewCam.PitchDegrees = viewportCameraRotation.Pitch;
	// Get the ground part of the transform by zeroing out the pitch (and roll, shouldn't happen but just in case).
	viewportCameraRotation.Pitch = 0.0;
	viewportCameraRotation.Roll = 0.0;
	viewCam.GroundTransform3D = FTransform(viewportCameraRotation, viewportCameraLocation);
	const FTransform4D cameraGroundTransform3Dbutin4D = FTransform4D::From3D(viewCam.GroundTransform3D);
	viewCam.ParentOfGroundTransform3Dtomakeit4D = cameraGroundTransform3Dbutin4D.TransformTo(viewCam.GroundTransform4D);
	// Update the rendering server's override camera transform
	// so that the editor camera can control the 4D view.
	UpdateRenderingServerWithEditorCameraTransform(InViewportClient);
}

void FUnreal4DEditorMode::SetEditorCameraRotation4Dand3D(FEditorViewportClient* InViewportClient, const FMatrix& InNewGroundBasis4D, const double InYawDegrees3D, const double InPitchDegrees3D)
{
	FRotator viewportCameraRotation = FRotator();
	viewportCameraRotation.Yaw = InYawDegrees3D;
	viewportCameraRotation.Pitch = InPitchDegrees3D;
	viewportCameraRotation.Roll = 0.0;
	InViewportClient->SetViewRotation(viewportCameraRotation);
	SetEditorCameraGroundBasis4D(InViewportClient, InNewGroundBasis4D);
}

void FUnreal4DEditorMode::SetEditorCameraFullTransform(FEditorViewportClient* InViewportClient, const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D)
{
	if (InViewportClient == nullptr)
	{
		return;
	}
	FEditorViewportCameraTransform4D& cameraTransform = GetOrAddViewportCameraTransform(InViewportClient);
	cameraTransform.GroundTransform4D = InCameraTransform4D;
	cameraTransform.GroundTransform3D = InCameraTransform3D;
	cameraTransform.ParentOfGroundTransform3Dtomakeit4D = FTransform4D::From3D(InCameraTransform3D).TransformTo(InCameraTransform4D);
	UpdateRenderingServerWithEditorCameraTransform(InViewportClient);
}

void FUnreal4DEditorMode::SetEditorCameraPosition4D(FEditorViewportClient* InViewportClient, const FVector4& InNewPosition)
{
	if (InViewportClient == nullptr)
	{
		return;
	}
	FEditorViewportCameraTransform4D& viewCam = GetOrAddViewportCameraTransform(InViewportClient);
	const FVector4 offset4D = InNewPosition - viewCam.GroundTransform4D.Origin;
	const FVector4 offsetInParentSpace = UBasis4D::Basis4D_InverseTransform(viewCam.ParentOfGroundTransform3Dtomakeit4D.Basis, offset4D);
	const FVector offset3D = FVector(offsetInParentSpace.X, offsetInParentSpace.Y, offsetInParentSpace.Z);
	const FVector newLoc = offset3D + InViewportClient->GetViewLocation();
	InViewportClient->SetViewLocation(newLoc);
	FRotator viewportCameraRotation = InViewportClient->GetViewRotation();
	viewCam.PitchDegrees = viewportCameraRotation.Pitch;
	// Ground transform excludes pitch/roll by design, to keep things simple and avoid gimbal lock issues.
	viewportCameraRotation.Pitch = 0.0;
	viewportCameraRotation.Roll = 0.0;
	viewCam.GroundTransform3D = FTransform(viewportCameraRotation, newLoc);
	const FVector4 offsetPerp = viewCam.ParentOfGroundTransform3Dtomakeit4D.Basis.TransformFVector4(FVector4(0.0, 0.0, 0.0, offsetInParentSpace.W));
	viewCam.ParentOfGroundTransform3Dtomakeit4D.Origin += offsetPerp;
	// Equivalent to: viewCam.ParentOfGroundTransform3Dtomakeit4D.Compose(FTransform4D::From3D(viewCam.GroundTransform3D));
	viewCam.GroundTransform4D.Origin = InNewPosition;
	UpdateRenderingServerWithEditorCameraTransform(InViewportClient);
}

bool FUnreal4DEditorMode::GetOverrideCursorVisibility(bool& bOutWantsOverride, bool& bOutHardwareCursorVisible, bool bInSoftwareCursorVisible) const
{
	// While the user is dragging the 4D gizmo, keep the hardware cursor visible so that
	// Alt+drag does not trigger Unreal's cursor-hiding behavior.
	if (UEditorTransformProxy4D::bIsTransformingAndIgnoringObjectTransformations)
	{
		bOutWantsOverride = true;
		bOutHardwareCursorVisible = true;
		return true;
	}
	return false;
}

bool FUnreal4DEditorMode::ReceivedFocus(FEditorViewportClient* InViewportClient, FViewport* InViewport)
{
	// Called when any viewport gains focus from the user clicking on it.
	// Ideally this would also work for tab switches, window focus, etc, but it doesn't.
	if (InViewportClient != nullptr && InViewport != nullptr)
	{
		SyncBlueprint4DTransformsWithArchetypes();
	}
	return false; // Don't consume the event, let it propagate to other modes.
}

void FUnreal4DEditorMode::OnDropElementTransformUpdated(AActor* InActor, const FTypedElementHandle& InElement, const FTransform& InTransform3D)
{
	if (InActor == nullptr)
	{
		return;
	}
	UScene4DComponent* root4DComponent = Cast<UScene4DComponent>(InActor->GetRootComponent());
	if (root4DComponent == nullptr)
	{
		return;
	}
	// Update the 4D transform from the 3D transform provided by the drag-and-drop operation.
	root4DComponent->SetLocalTransform4Dfrom3D(InTransform3D);
}

void FUnreal4DEditorMode::SyncBlueprint4DTransformsWithArchetypes()
{
	// Search all actors and sync all Blueprint instances with their archetypes.
	for (TActorIterator<AActor> actorIter(GWorld); actorIter; ++actorIter)
	{
		AActor* actor = *actorIter;
		if (actor == nullptr)
		{
			continue;
		}
		// For a tiny bit of efficiency gain, only sync 4D actors.
		// We expect that actors with 4D components will have a 4D root component.
		const UScene4DComponent* rootComponent4D = Cast<UScene4DComponent>(actor->GetRootComponent());
		if (rootComponent4D == nullptr)
		{
			continue;
		}
		// Sync all non-root 4D components in the actor. This only has an effect if they have an
		// archetype, meaning they are part of a Blueprint and not directly placed in the level.
		for (UActorComponent* component : actor->GetComponents())
		{
			UScene4DComponent* component4D = Cast<UScene4DComponent>(component);
			if (component4D == nullptr || component4D == rootComponent4D)
			{
				continue;
			}
			component4D->SyncTransform4DFromArchetype();
		}
	}
}
