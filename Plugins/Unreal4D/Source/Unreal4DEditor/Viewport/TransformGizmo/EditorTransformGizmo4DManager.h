#pragma once

#include "CoreMinimal.h"

class FEditorModeTools;
class FEditorViewportClient;
class UInteractiveGizmoManager;
class UScene4DComponent;
class UEditorTransformGizmo4D;

// Each editor mode owns one manager, including modes in Blueprint preview viewports.
class FEditorTransformGizmo4DManager
{
public:
	void Update(FEditorModeTools& InModeTools);
	void InvalidateSelection();
	void Shutdown(FEditorModeTools& InModeTools);

private:
	void DestroyGizmo();
	void RestoreViewportWidget();

	TWeakObjectPtr<UInteractiveGizmoManager> _gizmoManager;
	TWeakObjectPtr<UEditorTransformGizmo4D> _gizmo;
	TArray<TWeakObjectPtr<UScene4DComponent>> _selectedComponents;
	TWeakPtr<FEditorViewportClient> _viewportClient;
	bool _widgetHidden = false;
	TOptional<bool> _savedViewportShowWidget;
	int32 _widgetMode = INDEX_NONE;
	bool _preserveNonUniformScale = false;
	bool _rebuildRequested = true;
};
