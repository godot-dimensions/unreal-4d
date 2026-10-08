#pragma once

#include "Unreal4DEditorGizmoDefines.h"

#include "AssetEditorGizmoFactory.h"

#include "EditorTransformGizmo4DFactory.generated.h"

class UScene4DComponent;

// A factory class which creates the 4D transform gizmo using the registered builder.
// Both classes are required for integration in Unreal's interactive tools framework.
UCLASS(MinimalAPI)
class UEditorTransformGizmo4DFactory : public UObject, public IAssetEditorGizmoFactory
{
	GENERATED_BODY()

	static TMap<FEditorModeTools*, FEditorViewportClient*> GViewportClientsByModeTools;

	static FEditorTransformGizmo4DPartVisibilitySettings WidgetModeToVisibilitySettings(const UE::Widget::EWidgetMode InWidgetMode, const EEditorTransform4DKeepMode InKeepMode);
	static TArray<UScene4DComponent*> CollectSelected4DComponents(FEditorModeTools* InModeTools);

public:
	static void RegisterViewportClientForModeTools(FEditorModeTools* InModeTools, FEditorViewportClient* InViewportClient);
	static FEditorViewportClient* FindViewportClientForModeTools(FEditorModeTools* InModeTools);

	virtual bool CanBuildGizmoForSelection(FEditorModeTools* InModeTools) const override;
	virtual TArray<UInteractiveGizmo*> BuildGizmoForSelection(FEditorModeTools* InModeTools, UInteractiveGizmoManager* InGizmoManager) const override;
	virtual EAssetEditorGizmoFactoryPriority GetPriority() const override { return EAssetEditorGizmoFactoryPriority::High; }
	virtual void ConfigureGridSnapping(bool bInPositionGridEnabled, bool bInRotationGridEnabled, const TArray<UInteractiveGizmo*>& InGizmos) const override;
};
