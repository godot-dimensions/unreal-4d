#pragma once

#include "Unreal4DEditorDefines.h"

#include "Viewport/RotationWidget/EditorViewportRotationWidget4DManager.h"
#include "Viewport/TransformGizmo/EditorBlueprintSelectionNotifier4D.h"

#include "Delegates/IDelegateInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

class FEditorModeTools;
class FUnreal4DEditorMode;
class ILevelEditor;
class UScene4DComponent;

class FUnreal4DEditorModule : public IModuleInterface
{
public:
	static TObjectPtr<class UEditorTransformGizmo4DBuilder> TransformGizmo4DBuilder;
	static TObjectPtr<class UEditorTransformGizmo4DFactory> TransformGizmo4DFactory;

	void SetupGizmoForModeTools(FEditorModeTools& InModeTools);

private:
	void OnAssetEditorOpened(UObject* InAsset, IAssetEditorInstance* InEditorInstance);
	void OnLevelEditorCreated(TSharedPtr<ILevelEditor> InLevelEditor);
	void OnEditorBeginObjectMovement(UObject& InObject);
	void OnEditorEndObjectMovement(UObject& InObject);
	UScene4DComponent* ResolveObjectTo4DComponent(UObject& InObject);
	void Sync4DComponentFromEditorTransform(class UScene4DComponent* InComponent4D);

	TSet<FEditorModeTools*> ModeToolsWithGizmoFactory;
	FDelegateHandle LevelEditorCreatedHandle;
	FDelegateHandle AssetEditorOpenedHandle;
	FDelegateHandle BeginObjectMovementHandle;
	FDelegateHandle EndObjectMovementHandle;
	FDelegateHandle SelectionChangedHandle;
	TWeakObjectPtr<USelection> SelectedActorsPtr;
	FEditorBlueprintSelectionNotifier4D BlueprintSelectionNotifier;
	FEditorViewportRotationWidget4DManager ViewportRotationWidgetManager;

protected:
	//~ IModuleInterface BEGIN

	void StartupModule() override;
	void ShutdownModule() override;

	//~ IModuleInterface END
};
