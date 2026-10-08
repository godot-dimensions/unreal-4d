#pragma once

#include "Unreal4DEditorGizmoDefines.h"

#include "InteractiveGizmo.h"
#include "InteractiveGizmoBuilder.h"
#include "EditorTransformProxy4D.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"

class AEditorGizmoActor4D;
class FEditorModeTools;
class FEditorViewportClient;
class UClickDragInputBehavior;
class UMouseHoverBehavior;
class UGizmoViewContext;
struct FPropertyChangedEvent;

#include "EditorTransformGizmo4D.generated.h"

// Main class for the 4D transform gizmo, replacing the 3D transform gizmo for 4D objects.
// This is built by UEditorTransformGizmo4DBuilder, UEditorTransformGizmo4DFactory, and FUnreal4DEditorModule,
// and is registered with Unreal's Interactive Tools Framework to use those APIs.
// It uses a UEditorTransformProxy4D to manage the transforms of the selected objects
// and apply changes to them, and an AEditorGizmoActor4D for the visual representation
// of the gizmo in the editor viewport, which has several classes for the gizmo parts.
// Shared enums required for the gizmo are defined in Unreal4DEditorGizmoDefines.h.
UCLASS(MinimalAPI)
class UEditorTransformGizmo4D : public UInteractiveGizmo, public IClickDragBehaviorTarget, public IHoverBehaviorTarget
{
	GENERATED_BODY()

private:
	// Gizmo configuration and references, set by the builder/factory before use.
	UPROPERTY(Transient)
	TObjectPtr<UWorld> _world = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UEditorTransformProxy4D> _transformProxy4D = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UEditorTransformSnapSettings4D> _snapSettings = nullptr;
	FEditorModeTools* _modeTools = nullptr;
	FEditorViewportClient* _viewportClient = nullptr;
	FDelegateHandle _transformProxyChangedHandle;
	FDelegateHandle _editorViewportSettingsChangedHandle;
	FDelegateHandle _coordSystemChangedHandle;
	FEditorTransformGizmo4DPartVisibilitySettings _partVisibilitySettings;
	EEditorTransform4DKeepMode _keepMode = EEditorTransform4DKeepMode::Freeform;

	// Gizmo state, updated during interaction.
	FVector4 _transformReferenceValue = FVector4::Zero();
	EEditorTransformGizmo4DPart _currentTransformation = EEditorTransformGizmo4DPart::None;
	EEditorTransformGizmo4DPart _highlightedTransformation = EEditorTransformGizmo4DPart::None;
	double _gizmoScale = 1.0;

	// Gizmo visuals. Note that Unreal uses a threaded rendering system for editor gizmos,
	// so this actor needs to be protected with a mutex to prevent thread safety issues.
	UPROPERTY(Transient)
	TObjectPtr<AEditorGizmoActor4D> _visualActor = nullptr;

	// Input behaviors.
	UPROPERTY(Transient)
	TObjectPtr<UClickDragInputBehavior> _clickDragBehavior = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UMouseHoverBehavior> _hoverBehavior = nullptr;

	// View context for getting camera/viewport info.
	UPROPERTY(Transient)
	TObjectPtr<UGizmoViewContext> _gizmoViewContext = nullptr;

	static TWeakObjectPtr<UEditorTransformGizmo4D> _activeGizmo;

	static FString GizmoPartEnumToName(const EEditorTransformGizmo4DPart InPart);
	static bool IsGizmoPartForMove(const EEditorTransformGizmo4DPart InPart);
	static FORCEINLINE double ValidateScale(const double InScale);
	static FVector4 OriginAxisAlignedBivectorRingRaycast(const FVector4& InRayOrigin, const FVector4& InRayDirection, const FVector4& InAxis1, const FVector4& InAxis2, const FVector4& InPerp);
	FVector4 GetTransformRaycastValue(const FVector4 &InLocalRayOrigin, const FVector4 &InLocalRayDirection, const FMatrix &InLocalCameraMatrix) const;
	FTransform4D GetLocalTransformChange(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix) const;
	EEditorTransformGizmo4DPart CheckForBestHit(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FVector4& InLocalPerpDirection) const;
	void SetHighlightedPart(const EEditorTransformGizmo4DPart InNewHighlightedPart);

	void BeginTransformation(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix);
	void ProcessTransformation(const FVector4& InLocalRayOrigin, const FVector4& InLocalRayDirection, const FMatrix& InLocalCameraMatrix);
	void HandleEditorViewportSettingsChanged(UObject* InObject, FPropertyChangedEvent& InPropertyChangedEvent);
	void HandleCoordSystemChanged(ECoordSystem InCoordSystem);
	void ReadFromEditorViewportSettings();
	FTransform4D GetEditorCameraTransform4DForViewport() const;

	// Helper to convert world ray to local 4D ray data with internal scale handling.
	void GetLocalRayDataWithScale(const FInputDeviceRay& InDeviceRay, FVector4& OutLocalRayOrigin, FVector4& OutLocalRayDirection, FMatrix& OutLocalCameraMatrix) const;

public:
	UEditorTransformGizmo4D() {}

	// InteractiveGizmo interface.
	virtual void Setup() override;
	virtual void Shutdown() override;

	// 4D-specific configuration by the builder and factory.
	void SetWorld(UWorld* InWorld) { _world = InWorld; }
	void Setup4D(const TArray<UScene4DComponent*>& InObjects4D, const FEditorTransformGizmo4DPartVisibilitySettings InVisibilitySettings, const ECoordSystem InCoordSystem, FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient = nullptr);
	bool DuplicateSelectedObjects();
	static bool DuplicateSelectedObjectsStatic();

	// IClickDragBehaviorTarget interface.
	virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& InPressPos) override;
	virtual void OnClickPress(const FInputDeviceRay& InPressPos) override;
	virtual void OnClickDrag(const FInputDeviceRay& InDragPos) override;
	virtual void OnClickRelease(const FInputDeviceRay& InReleasePos) override;
	virtual void OnTerminateDragSequence() override;

	// IHoverBehaviorTarget interface.
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& InPressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& InDevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& InDevicePos) override;
	virtual void OnEndHover() override;
};

UCLASS(MinimalAPI)
class UEditorTransformGizmo4DBuilder : public UInteractiveGizmoBuilder
{
	GENERATED_BODY()

public:
	virtual UInteractiveGizmo* BuildGizmo(const FToolBuilderState& SceneState) const override;
};
