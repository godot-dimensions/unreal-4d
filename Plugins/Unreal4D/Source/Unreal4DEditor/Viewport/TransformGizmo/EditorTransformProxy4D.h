#pragma once

#include "Unreal4DEditorGizmoDefines.h"

#include "Templates/UniquePtr.h"
#include "ScopedTransaction.h"

#include "../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "EditorTransformSnapSettings4D.h"

#include "EditorTransformProxy4D.generated.h"

class FEditorModeTools;
class FEditorViewportClient;

DECLARE_MULTICAST_DELEGATE_OneParam(FOn4DTransformProxyChanged, const FTransform4D&);

// Calculates relative transforms of selected 4D objects, and can re-apply
// those transforms to them after this object has itself been transformed.
// In Unreal, see UTransformProxy for reference.
// In Godot 4D, this functionality is built into EditorTransformGizmo4D.
UCLASS(Transient, MinimalAPI)
class UEditorTransformProxy4D : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UScene4DComponent*> _objects4D;
	TArray<FTransform4D> _oldObjectGlobalTransforms4D;
	FTransform4D _currentProxyTransform4D;
	FTransform4D _oldProxyTransform4D;
	FEditorModeTools* _modeTools = nullptr;
	FEditorViewportClient* _viewportClient = nullptr;
	FVector4 _cameraFollowOriginalPosition4D = FVector4::Zero();
	FVector4 _cameraFollowRelativeOffset4D = FVector4::Zero();
	ECoordSystem _coordSystem  = ECoordSystem::COORD_Local;
	EEditorTransform4DKeepMode _keepMode = EEditorTransform4DKeepMode::Freeform;
	bool _bCameraFollowShiftWasDown = false;
	TUniquePtr<FScopedTransaction> _activeTransformTransaction;

	static FTransform4D GetParentGlobalTransform(const UScene4DComponent* InObj4D);
	static bool IsShiftDownInEditor();
	inline void ResetFollowState();
	class FUnreal4DEditorMode* GetEditorMode() const;
	FVector4 GetEditorCameraPosition4D() const;
	void SetEditorCameraPosition4D(const FVector4& InNewPosition) const;

public:
	FOn4DTransformProxyChanged On4DTransformProxyChanged;
	static FTransform4D LatestProxyTransformStatic;
	static bool bIsTransformingAndIgnoringObjectTransformations;

	UPROPERTY(Transient)
	TObjectPtr<UEditorTransformSnapSettings4D> SnapSettings = nullptr;

	bool DuplicateSelectedObjects();

	void SetEditorContext(FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient);
	FVector4 GetShiftSlideOffset();
	void SetCoordSystem(ECoordSystem InCoordSystem);
	void SetKeepMode(EEditorTransform4DKeepMode InKeepMode) { _keepMode = InKeepMode; }
	void Select4DObjects(const TArray<UScene4DComponent*>& InObjects4D);
	void UpdateProxyTransformFromObjects();
	UFUNCTION()
	void OnComponent4DTransformChanged(UScene4DComponent* InChangedComponent);
	void ApplyTransformChange(const FTransform4D& InTransformChange, const bool bInUseKeepMode = true);

	void BeginTransformations();
	void CancelTransformations();
	void CommitTransformations();

	FTransform4D GetCurrentTransform4D() const { return _currentProxyTransform4D; }
	FTransform4D GetOldTransform4D() const { return _oldProxyTransform4D; }
};
