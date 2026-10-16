#pragma once

#include "Unreal4DEditorGizmoDefines.h"

#include "UObject/Object.h"

#include "EditorTransformGizmo4DFactory.generated.h"

class UScene4DComponent;
class FEditorViewportClient;
class FEditorModeTools;
class UInteractiveGizmo;
class UInteractiveGizmoManager;

// A factory class which creates the 4D transform gizmo using the registered builder.
// Both classes are required for integration in Unreal's interactive tools framework.
UCLASS(MinimalAPI)
class UEditorTransformGizmo4DFactory : public UObject
{
	GENERATED_BODY()

	friend class FEditorTransformGizmo4DManager;
	static TMap<FEditorModeTools*, TWeakPtr<FEditorViewportClient>> GViewportClientsByModeTools;

	static FEditorTransformGizmo4DPartVisibilitySettings WidgetModeToVisibilitySettings(const UE::Widget::EWidgetMode InWidgetMode, const EEditorTransform4DKeepMode InKeepMode);
	static TArray<UScene4DComponent*> CollectSelected4DComponents(FEditorModeTools* InModeTools);

public:
	static const FString GizmoIdentifier;
	static void RegisterViewportClientForModeTools(FEditorModeTools* InModeTools, TSharedPtr<FEditorViewportClient> InViewportClient);
	static TSharedPtr<FEditorViewportClient> FindViewportClientForModeTools(FEditorModeTools* InModeTools);

	bool CanBuildGizmoForSelection(FEditorModeTools* InModeTools) const;
	TArray<UInteractiveGizmo*> BuildGizmoForSelection(FEditorModeTools* InModeTools, UInteractiveGizmoManager* InGizmoManager) const;
	void ConfigureGridSnapping(bool bInPositionGridEnabled, bool bInRotationGridEnabled, const TArray<UInteractiveGizmo*>& InGizmos) const;
};
