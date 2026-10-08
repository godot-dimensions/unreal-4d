#include "EditorBlueprintSelectionNotifier4D.h"

#include "../../../Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "EditorTransformGizmo4DFactory.h"
#include "../../Unreal4DEditorModule.h"

// Unreal includes.
#include "AssetEditorModeManager.h"
#include "BlueprintEditor.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "Elements/Component/ComponentElementData.h"
#include "Features/IModularFeatures.h"
#include "SEditorViewport.h"
#include "Selection.h"
#include "SSCSEditorViewport.h"

// Supress this warning because there is no alternative API.
// warning C4996: 'IBlueprintSelectionNotifier': This API is internal to the engine. As such, there is no guarantee that it won't change or be removed in future releases. Please use at your own risk.
PRAGMA_DISABLE_DEPRECATION_WARNINGS

void FEditorBlueprintSelectionNotifier4D::Register()
{
	IModularFeatures::Get().RegisterModularFeature(IBlueprintSelectionNotifier::GetModularFeatureName(), this);
	// When the SCS preview is reconstructed, component instances are replaced, and we need to
	// rebind the gizmo's transform proxy to the new instances. Use OnObjectsReplaced to detect this.
	_onObjectsReplacedHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddLambda(
		[this](const TMap<UObject*, UObject*>& InReplacementMap)
		{
			this->OnObjectsReplaced(InReplacementMap);
		}
	);
}

void FEditorBlueprintSelectionNotifier4D::Unregister()
{
	IModularFeatures::Get().UnregisterModularFeature(IBlueprintSelectionNotifier::GetModularFeatureName(), this);
	// Unsubscribe from any active SCS session reconstruction events.
	if (_onObjectsReplacedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReplaced.Remove(_onObjectsReplacedHandle);
	}
	_viewportModeTools = nullptr;
	_selectedComponentNames.Empty();
}

void FEditorBlueprintSelectionNotifier4D::OnBlueprintSelectionUpdated(IAssetEditorInstance* InAssetEditorInstance, UBlueprint* InBlueprint, const TArray<FTypedElementHandle>& InSelectedElements)
{
	// Can't use Cast<> here because FBlueprintEditor is not UObject-derived.
	FBlueprintEditor* blueprintEditor = static_cast<FBlueprintEditor*>(InAssetEditorInstance);
	if (blueprintEditor == nullptr) { return; }
	// SCS stands for "Simple Construction Script". Contrary to its name, it's not
	// that simple, though it is simple relative to the "Complex Construction Script"
	// (CCS) system. Also, despite "script", it isn't the visual scripting system either.
	// SCS is the part of Blueprint that allows defining component hierarchies.
	// SSCS stands for "Slate SCS", which is the UI of the SCS editor viewport.
	TSharedPtr<SSCSEditorViewport> viewport = blueprintEditor->GetSubobjectViewport();
	if (!viewport.IsValid()) { return; }
	TSharedPtr<FEditorViewportClient> viewportClient = viewport->GetViewportClient();
	if (!viewportClient.IsValid()) { return; }
	_viewportModeTools = viewportClient->GetModeTools();
	if (_viewportModeTools == nullptr) { return; }
	UEditorTransformGizmo4DFactory::RegisterViewportClientForModeTools(_viewportModeTools, viewportClient.Get());
	// The SCS viewport auto-creates an FAssetEditorModeManager but never calls
	// `SetPreviewScene()` on it, so when `AEditorGizmoActor4D` calls `GetWorld()`,
	// it gets the main Level editor world instead of the Blueprint preview world.
	// We need to call `SetPreviewScene()` so the gizmo spawns in the correct world.
	FPreviewScene* previewScene = viewportClient->GetPreviewScene();
	if (previewScene != nullptr)
	{
		FAssetEditorModeManager* assetModeManager = static_cast<FAssetEditorModeManager*>(_viewportModeTools);
		if (assetModeManager->GetPreviewScene() == nullptr)
		{
			assetModeManager->SetPreviewScene(previewScene);
		}
	}
	// Ensure gizmo is set up on this viewport's FEditorModeTools.
	FUnreal4DEditorModule& unreal4DEditorModule = FModuleManager::GetModuleChecked<FUnreal4DEditorModule>("Unreal4DEditor");
	unreal4DEditorModule.SetupGizmoForModeTools(*_viewportModeTools);
	// FSCSEditorViewportClient initializes its local WidgetMode to WM_Translate in the constructor,
	// but never calls ModeTools->SetWidgetMode(), so we need to do that to fix the gizmo.
	_viewportModeTools->SetWidgetMode(viewportClient->GetWidgetMode());
	// FSCSEditorViewportClient also owns a local WidgetCoordSystem value; mirror it into
	// ModeTools so gizmos using ModeTools->GetCoordSystem() stay in sync in Blueprint SCS.
	_viewportModeTools->SetCoordSystem(viewportClient->GetWidgetCoordSystemSpace());
	// Populate the viewport's FEditorModeTools component selection from the Blueprint selection.
	// The SCS viewport's FAssetEditorModeManager has its own private USelection sets which
	// are normally empty. We fill them here so the TransformGizmo4DFactory can read them.
	_selectedComponentNames.Reset();
	USelection* selectedComponents = _viewportModeTools->GetSelectedComponents();
	if (!IsValid(selectedComponents) || !selectedComponents->IsValidLowLevel())
	{
		_viewportModeTools = nullptr; // Avoid future access to this object which points to destroyed memory.
		return;
	}
	if (!selectedComponents->CanSelectAnything())
	{
		return; // Can sometimes happen on the first tick, do not set _viewportModeTools to nullptr.
	}
	selectedComponents->BeginBatchSelectOperation();
	selectedComponents->DeselectAll();
	for (const FTypedElementHandle& Handle : InSelectedElements)
	{
		UActorComponent* Component = ComponentElementDataUtil::GetComponentFromHandle(Handle, /*bSilent=*/ true);
		if (Component != nullptr)
		{
			selectedComponents->Select(Component);
		}
	}
	selectedComponents->EndBatchSelectOperation();
	_viewportModeTools->ActorSelectionChangeNotify();
	// Suppress the standard 3D FWidget on this viewport when 4D components are selected.
	// UGizmoEdMode::Enter() calls SetShowWidget(false), but that only sets the ModeTools flag.
	// FEditorViewportClient has its own bShowWidget used for deciding 3D gizmo visibility.
	// We need to set BOTH so the Unreal 3D FWidget doesn't draw on top of the 4D gizmo.
	bool bHas4DSelection = false;
	TArray<UActorComponent*> selectedActorComponents;
	selectedComponents->GetSelectedObjects<UActorComponent>(selectedActorComponents);
	for (UActorComponent* actorComp : selectedActorComponents)
	{
		// Collect the currently selected component names. We store names (not pointers) because
		// SCS reconstruction destroys and recreates component instances, invalidating weak pointers.
		// We'll match these names against the replacement map when OnObjectsReplaced fires.
		// In case this tracking is not desired, comment out only this line to disable it.
		_selectedComponentNames.Add(actorComp->GetPathName());
		// Check if the component is a 4D component.
		UScene4DComponent* comp4D = Cast<UScene4DComponent>(actorComp);
		if (comp4D != nullptr)
		{
			bHas4DSelection = true;
		}
	}
	// Show the 3D widget whenever the 4D one is not being shown.
	viewportClient->ShowWidget(!bHas4DSelection);
}

void FEditorBlueprintSelectionNotifier4D::OnObjectsReplaced(const TMap<UObject*, UObject*>& InReplacementMap)
{
	// This callback fires when SCS component instances are replaced during preview reconstruction.
	// Match new components by name to rebind the gizmo to the new instances.
	if (_viewportModeTools == nullptr)
	{
		return;
	}
	USelection* selectedComponents = _viewportModeTools->GetSelectedComponents();
	if (!IsValid(selectedComponents) || !selectedComponents->IsValidLowLevel())
	{
		_viewportModeTools = nullptr; // Avoid future access to this object which points to destroyed memory.
		return;
	}
	if (!selectedComponents->CanSelectAnything())
	{
		return; // Can sometimes happen on the first tick, do not set _viewportModeTools to nullptr.
	}
	// Check if any of our selected components were replaced.
	TArray<UActorComponent*> newComponents;
	// Match new components by name against our stored names, if any. We use names instead of pointers
	// because SCS reconstruction destroys and recreates all component instances, making
	// weak pointers to the old instances invalid before OnObjectsReplaced fires.
	if (_selectedComponentNames.Num() > 0)
	{
		for (const TPair<UObject*, UObject*>& pair : InReplacementMap)
		{
			UActorComponent* newComp = Cast<UActorComponent>(pair.Value);
			if (newComp == nullptr)
			{
				continue;
			}
			const FString newCompName = newComp->GetPathName();
			if (_selectedComponentNames.Contains(newCompName))
			{
				newComponents.Add(newComp);
			}
		}
	}
	// Reselect the new components, even if that means none get selected.
	// This is necessary to prevent the gizmo from receiving stale pointers to the old components.
	selectedComponents->BeginBatchSelectOperation();
	TArray<UActorComponent*> currentComponents;
	selectedComponents->GetSelectedObjects<UActorComponent>(currentComponents);
	for (UActorComponent* comp : currentComponents)
	{
		selectedComponents->Deselect(comp);
	}
	for (UActorComponent* newComp : newComponents)
	{
		selectedComponents->Select(newComp);
	}
	selectedComponents->EndBatchSelectOperation();
	_viewportModeTools->ActorSelectionChangeNotify();
	// Update our cached component list to the new instances (by path name).
	_selectedComponentNames.Reset();
	for (UActorComponent* newComp : newComponents)
	{
		_selectedComponentNames.Add(newComp->GetPathName());
	}
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS
