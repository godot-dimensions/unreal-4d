#pragma once

#include "../Unreal4DEditorGizmoDefines.h"

#include "../../../../Unreal4DCore/Scene/Actors/Scene4DActor.h"
#include "BaseGizmos/GizmoActor.h"

#include "EditorTransformGizmoActor4D.generated.h"

class UGizmoViewContext;
class UScene4DComponent;
class UEditorTransformGizmoBaseMesh4D;
class FEditorModeTools;
class FEditorViewportClient;

// Actor that provides threaded visualization of the gizmo in the Unreal editor.
// This extends AGizmoActor for baseline integration with the InteractiveToolsFramework,
// for example making sure it does not display in the hierarchy outliner, but most of the
// functionality is custom due to the special needs of 4D and limitations in the base classes.
// This actor is entirely downstream of the gizmo - the gizmo provides data to it,
// but no data is read back out of this actor to the gizmo.
UCLASS(MinimalAPI)
class AEditorGizmoActor4D : public AGizmoActor
{
	GENERATED_BODY()

private:
	// Start out the camera transform with an invalid value to ensure the first update goes through.
	FTransform4D _cameraTransform4D = FTransform4D(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	FTransform _cameraTransform3D = FTransform::Identity;
	FEditorModeTools* _modeTools = nullptr;
	FEditorViewportClient* _viewportClient = nullptr;
	UPROPERTY(Transient)
	TMap<EEditorTransformGizmo4DPart, TObjectPtr<UEditorTransformGizmoBaseMesh4D>> _gizmoParts4D;
	UPROPERTY(Transient)
	TObjectPtr<UScene4DComponent> _root4DComponent = nullptr;

	template<typename T>
	T* CreatePart();

	bool TryUpdateCameraTransform4D();
	void UpdatePartMeshes();

protected:
	virtual void Tick(float InDeltaTime) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

public:
	void SetupAndGenerateParts(FEditorTransformGizmo4DPartVisibilitySettings InVisibilitySettings, UWorld* InWorld, UGizmoViewContext* InGizmoViewContext, FEditorModeTools* InModeTools = nullptr, FEditorViewportClient* InViewportClient = nullptr);
	void SetGizmoTransform4D(const FTransform4D& InTransform);
	void SetPartHighlighted(EEditorTransformGizmo4DPart InPart, bool bInHighlight);

	AEditorGizmoActor4D();
};
