#pragma once

#include "../../Unreal4DEditorDefines.h"

// Supress this warning because there is no alternative API.
// warning C4996: 'IBlueprintSelectionNotifier': This API is internal to the engine. As such, there is no guarantee that it won't change or be removed in future releases. Please use at your own risk.
PRAGMA_DISABLE_DEPRECATION_WARNINGS

#include "IBlueprintSelectionNotifier.h"

class UBlueprint;
class IAssetEditorInstance;
struct FTypedElementHandle;
class FEditorModeTools;

// Bridges Blueprint SCS component selection to the SCS viewport's FEditorModeTools
// so that UGizmoEdMode::RecreateGizmo() fires and our TransformGizmo4DFactory gets queried.
//
// The Blueprint editor's SCS viewport creates its own FAssetEditorModeManager with
// private (empty) USelection sets and never calls ActorSelectionChangeNotify().
// This notifier fills that gap.
//
// Also handles SCS preview reconstruction: When components are replaced during SCS
// reconstruction, this notifier rebinds the gizmo proxy to the new component instances
// so the gizmo continues to follow transform changes.
class FEditorBlueprintSelectionNotifier4D : public IBlueprintSelectionNotifier
{
public:
	void Register();
	void Unregister();
	virtual ~FEditorBlueprintSelectionNotifier4D() = default;

	// IBlueprintSelectionNotifier.
	virtual void OnBlueprintSelectionUpdated(IAssetEditorInstance* InAssetEditorInstance, UBlueprint* InBlueprint, const TArray<FTypedElementHandle>& InSelectedElements) override;

private:
	// Track the current SCS context to handle reconstruction rebinding.
	FEditorModeTools* _viewportModeTools = nullptr;
	TArray<FString> _selectedComponentNames; // Store names, not pointers - pointers get destroyed during SCS reconstruction
	FDelegateHandle _onObjectsReplacedHandle;

	// Called when component instances are replaced during SCS reconstruction.
	void OnObjectsReplaced(const TMap<UObject*, UObject*>& InReplacementMap);
};

PRAGMA_ENABLE_DEPRECATION_WARNINGS
