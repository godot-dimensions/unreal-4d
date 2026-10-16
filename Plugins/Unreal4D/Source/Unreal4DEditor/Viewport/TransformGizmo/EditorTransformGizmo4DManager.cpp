#include "EditorTransformGizmo4DManager.h"

#include "EditorTransformGizmo4D.h"
#include "EditorTransformGizmo4DFactory.h"
#include "../../Unreal4DEditorModule.h"
#include "../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "InteractiveGizmoManager.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "Tools/EdModeInteractiveToolsContext.h"

void FEditorTransformGizmo4DManager::Update(FEditorModeTools& InModeTools)
{
	UModeManagerInteractiveToolsContext* context = InModeTools.GetInteractiveToolsContext();
	UInteractiveGizmoManager* gizmoManager = context ? context->GizmoManager : nullptr;
	UEditorTransformGizmo4DFactory* factory = FUnreal4DEditorModule::TransformGizmo4DFactory;
	if (gizmoManager == nullptr || factory == nullptr)
	{
		Shutdown(InModeTools);
		return;
	}
	const TArray<UScene4DComponent*> selectedComponents = factory->CollectSelected4DComponents(&InModeTools);
	if (selectedComponents.IsEmpty())
	{
		Shutdown(InModeTools);
		return;
	}

	TArray<TWeakObjectPtr<UScene4DComponent>> selection;
	for (UScene4DComponent* component : selectedComponents)
	{
		selection.Add(component);
	}
	const ULevelEditorViewportSettings* settings = GetDefault<ULevelEditorViewportSettings>();
	const int32 widgetMode = static_cast<int32>(InModeTools.GetWidgetMode());
	const bool needsRebuild = _rebuildRequested || !_gizmo.IsValid() || _gizmoManager.Get() != gizmoManager
			|| selection != _selectedComponents || widgetMode != _widgetMode
			|| settings->PreserveNonUniformScale != _preserveNonUniformScale
			|| gizmoManager->FindGizmoByInstanceIdentifier(factory->GizmoIdentifier) != _gizmo.Get();
	if (needsRebuild)
	{
		DestroyGizmo();
		_gizmoManager = gizmoManager;
		const TArray<UInteractiveGizmo*> gizmos = factory->BuildGizmoForSelection(&InModeTools, gizmoManager);
		if (!gizmos.IsEmpty())
		{
			_gizmo = Cast<UEditorTransformGizmo4D>(gizmos[0]);
		}
		_selectedComponents = MoveTemp(selection);
		_widgetMode = widgetMode;
		_preserveNonUniformScale = settings->PreserveNonUniformScale;
		_rebuildRequested = false;
	}
	if (!_gizmo.IsValid())
	{
		Shutdown(InModeTools);
		return;
	}

	// GetShowWidget includes selection-dependent checks, so it cannot snapshot the mode's visibility flag.
	InModeTools.SetShowWidget(false);
	_widgetHidden = true;
	const TSharedPtr<FEditorViewportClient> viewportClient = factory->FindViewportClientForModeTools(&InModeTools);
	if (_viewportClient.Pin() != viewportClient)
	{
		RestoreViewportWidget();
		_viewportClient = viewportClient;
	}
	if (viewportClient.IsValid())
	{
		if (!_savedViewportShowWidget.IsSet())
		{
			_savedViewportShowWidget = viewportClient->GetShowWidget();
		}
		viewportClient->ShowWidget(false);
	}
	_gizmo->ConfigureGridSnapping(settings->GridEnabled, settings->RotGridEnabled);
}

void FEditorTransformGizmo4DManager::DestroyGizmo()
{
	UInteractiveGizmoManager* gizmoManager = _gizmoManager.Get();
	UEditorTransformGizmo4D* gizmo = _gizmo.Get();
	if (gizmoManager && gizmo && gizmoManager->FindGizmoByInstanceIdentifier(UEditorTransformGizmo4DFactory::GizmoIdentifier) == gizmo)
	{
		gizmoManager->DestroyGizmo(gizmo);
	}
	_gizmo.Reset();
}

void FEditorTransformGizmo4DManager::InvalidateSelection()
{
	// Selection notifications can precede a preview rebuild. Release references before it replaces components.
	DestroyGizmo();
	_rebuildRequested = true;
}

void FEditorTransformGizmo4DManager::RestoreViewportWidget()
{
	if (_savedViewportShowWidget.IsSet())
	{
		if (const TSharedPtr<FEditorViewportClient> viewportClient = _viewportClient.Pin())
		{
			viewportClient->ShowWidget(_savedViewportShowWidget.GetValue());
		}
	}
	_savedViewportShowWidget.Reset();
	_viewportClient.Reset();
}

void FEditorTransformGizmo4DManager::Shutdown(FEditorModeTools& InModeTools)
{
	DestroyGizmo();
	_gizmoManager.Reset();
	_selectedComponents.Reset();
	_rebuildRequested = true;
	RestoreViewportWidget();
	if (_widgetHidden)
	{
		InModeTools.SetShowWidget(true);
		_widgetHidden = false;
	}
}
