#include "Unreal4DEditorLibrary.h"

#include "EditorModeManager.h"
#include "LevelEditorViewport.h"
#include "Unreal4DEditorMode.h"
#include "UnrealEdGlobals.h"

void UUnreal4DEditorLibrary::SetEditorCameraFullTransform(const FTransform4D& InCameraTransform4D, const FTransform& InCameraTransform3D)
{
	FLevelEditorViewportClient* viewportClient = GCurrentLevelEditingViewportClient;
	if (viewportClient == nullptr)
	{
		return;
	}
	FEditorModeTools& modeTools = GLevelEditorModeTools();
	FUnreal4DEditorMode* editorMode = static_cast<FUnreal4DEditorMode*>(modeTools.GetActiveMode(FUnreal4DEditorMode::ModeID));
	if (editorMode != nullptr)
	{
		editorMode->SetEditorCameraFullTransform(viewportClient, InCameraTransform4D, InCameraTransform3D);
	}
}
